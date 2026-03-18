#pragma once
#include <cstdint>

#include "address.h"

namespace tickwire
{
    class Socket // Responsible for RAII (Resource Acquisition is Initialization)
    {
    public:
        Socket();
        ~Socket();

        //Non-Copyable (sockets should remain unique resources)
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        //Movable
        Socket(Socket&&) noexcept;
        Socket& operator=(Socket&&) noexcept;

        bool open_udp(std::uint16_t port);
        void close();

        bool is_valid() const noexcept;

        // Receive raw bytes into provided buffer.
        // Returns number of bytes received, or -1 on failure.
        bool receive(uint8_t* buffer, size_t buffer_size, TickWireAddress& sender, size_t& received);

        // Send raw bytes to destination address.
        // Returns number of bytes sent, or -1 on failure
        int send(const void* data, int size, const void* addr, int addr_len);

    private:
        int handle_{-1};
    };
}
