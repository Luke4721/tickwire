#pragma once

#include <cstdint>
#include <array>


namespace tickwire
{
    // Fixed-size,  trivially copyable representation of a UDP endpoint

    struct TickWireAddress
    {
        // Always 16 bytes so IPv4 fits inside IPv6-mapped space.
        std::array<uint8_t,16> ip{};

        // stored in network byte order
        std::uint16_t port{0};

        // Address family (AF_INET or AF_INET6)
        std::uint8_t family{0};

        // Padding to keep structure aligned (and predictable size)
        std::uint8_t  padding{0};
    };

    static_assert(std::is_trivially_copyable_v<TickWireAddress>,
        "TickWireAddress must be trivially copyable");


    // Convert from OS sockaddr storage into TickWireAddress.
    bool from_sockaddr(const void* addr,int addr_len, TickWireAddress& out);

    // Convert TickWireAddress back into OS sockaddr form (for send).
    bool to_sockaddr(const TickWireAddress& in, void* addr, int& addr_len);

}

