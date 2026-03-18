#include "tickwire/socket.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

namespace tickwire
{

    //We need to initialize Winsock once per process
    static bool ensure_winsock_initialized()
    {
        static bool initialized = false;

        if (!initialized)
        {
            WSADATA data{};
            if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
            {
                return false;
            }
            initialized = true;
        }
        return true;
    }
    bool Socket::open_udp(std::uint16_t port) {
        if (!ensure_winsock_initialized()) {
            return false;
        }

        SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            handle_ = -1;
            return false;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
            ::closesocket(sock);
            handle_ = -1;
            return false;
        }


        handle_ = static_cast<int>(sock);
        u_long mode{1};
        ioctlsocket(sock,FIONBIO,&mode);
        return true;
    }

    bool Socket::receive(uint8_t* buffer,
                     std::size_t buffer_size,
                     TickWireAddress& sender,
                     std::size_t& received)
    {
        sockaddr_storage addr{};
        int addr_len = sizeof(addr);

        int result = recvfrom(handle_,
                              reinterpret_cast<char*>(buffer),
                              static_cast<int>(buffer_size),
                              0,
                              reinterpret_cast<sockaddr*>(&addr),
                              &addr_len);

        if (result <= 0)
            return false;

        received = static_cast<std::size_t>(result);

        from_sockaddr(reinterpret_cast<sockaddr*>(&addr),
                      addr_len,
                      sender);

        return true;
    }

    int Socket::send(const void* data, int size, const void* addr, int addr_len)
    {
        if (handle_ == -1)
        {
            return -1;
        }

        int sent = ::sendto(static_cast<SOCKET>(handle_),
            static_cast<const char*>(data),
            size,
            0,
            reinterpret_cast<const sockaddr*>(addr),
            addr_len);

        if (sent == SOCKET_ERROR)
        {
            return -1;
        }
        return sent;
    }


    void Socket::close() {
        if (handle_ != -1) {
            ::closesocket(static_cast<SOCKET>(handle_));
            handle_ = -1;
        }
    }

} // namespace tickwire
