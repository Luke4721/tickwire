#pragma once

#include <cstdint>
#include "address.h"

namespace tickwire
{
    // Socket wraps a platform-specific UDP socket handle.
    // Owns the handle exclusively — non-copyable, movable.
    // Platform implementations live in src/platform/.
    class Socket
    {
    public:
        Socket() noexcept;
        ~Socket();

        // Non-copyable: sockets are unique OS resources.
        Socket(const Socket&)            = delete;
        Socket& operator=(const Socket&) = delete;

        // Movable: ownership can be transferred.
        Socket(Socket&&) noexcept;
        Socket& operator=(Socket&&) noexcept;

        // Bind a UDP socket on the given port. Returns false on failure.
        bool open_udp(std::uint16_t port) noexcept;

        // Close and invalidate the socket.
        void close() noexcept;

        // True if the handle is valid and the socket is open.
        bool is_valid() const noexcept;

        // Receive raw bytes into buffer. Fills addr/addr_len with sender info.
        // Returns bytes received, or -1 on failure / no data (non-blocking).
        int receive(void* buffer, int buffer_size, void* addr, int* addr_len) noexcept;

        // Send raw bytes to destination addr.
        // Returns bytes sent, or -1 on failure.
        int send(const void* data, int size, const void* addr, int addr_len) noexcept;

    private:
        // FIX: use uintptr_t instead of int.
        // On 64-bit Windows, SOCKET is UINT_PTR (64-bit). Storing it in an int
        // silently truncates the upper 32 bits, corrupting high-value handles.
        // uintptr_t matches the platform pointer size on all targets.
        uintptr_t handle_{static_cast<uintptr_t>(-1)};

        // Sentinel value meaning "no socket open". Matches INVALID_SOCKET on
        // Windows and -1 cast to uintptr_t on POSIX.
        static constexpr uintptr_t INVALID_HANDLE = static_cast<uintptr_t>(-1);
    };

} // namespace tickwire