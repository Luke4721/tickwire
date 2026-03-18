#include "tickwire/socket.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mutex>

namespace tickwire
{
    // FIX: use std::call_once to make Winsock initialization thread-safe.
    // The original used a plain bool with no synchronization — two threads
    // calling open_udp() simultaneously would both read false, both call
    // WSAStartup(), and both write true: a data race on a non-atomic variable.
    static bool ensure_winsock_initialized() noexcept
    {
        static std::once_flag flag;
        static bool           success = false;

        std::call_once(flag, [] {
            WSADATA data{};
            success = (WSAStartup(MAKEWORD(2, 2), &data) == 0);
        });

        return success;
    }

    bool Socket::open_udp(std::uint16_t port) noexcept
    {
        if (!ensure_winsock_initialized())
            return false;

        SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET)
        {
            handle_ = INVALID_HANDLE;
            return false;
        }

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(port);

        if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            ::closesocket(sock);
            handle_ = INVALID_HANDLE;
            return false;
        }

        // Set non-blocking mode
        u_long mode = 1;
        ioctlsocket(sock, FIONBIO, &mode);

        // FIX: store as uintptr_t, not int.
        // SOCKET is UINT_PTR on 64-bit Windows (8 bytes).
        // Casting to int truncates the upper 32 bits, silently corrupting
        // the handle for any high-valued socket descriptor.
        handle_ = static_cast<uintptr_t>(sock);
        return true;
    }

    int Socket::receive(void* buffer, int buffer_size, void* addr, int* addr_len) noexcept
    {
        if (handle_ == INVALID_HANDLE)
            return -1;

        return recvfrom(
            static_cast<SOCKET>(handle_),
            static_cast<char*>(buffer),
            buffer_size,
            0,
            reinterpret_cast<sockaddr*>(addr),
            addr_len
        );
    }

    int Socket::send(const void* data, int size, const void* addr, int addr_len) noexcept
    {
        if (handle_ == INVALID_HANDLE)
            return -1;

        int sent = ::sendto(
            static_cast<SOCKET>(handle_),
            static_cast<const char*>(data),
            size,
            0,
            reinterpret_cast<const sockaddr*>(addr),
            addr_len
        );

        return (sent == SOCKET_ERROR) ? -1 : sent;
    }

    void Socket::close() noexcept
    {
        if (handle_ != INVALID_HANDLE)
        {
            ::closesocket(static_cast<SOCKET>(handle_));
            handle_ = INVALID_HANDLE;
        }
    }

} // namespace tickwire