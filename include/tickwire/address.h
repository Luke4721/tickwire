#pragma once

#include <cstdint>
#include <array>

namespace tickwire
{
    // Strongly-typed address family — replaces raw AF_INET / AF_INET6 integers.
    enum class AddressFamily : uint8_t
    {
        Unknown = 0,
        IPv4    = 4,
        IPv6    = 6,
    };

    // Fixed-size, trivially copyable representation of a UDP endpoint.
    // Always 16 bytes for the IP so IPv4 fits inside IPv6-mapped space.
    struct TickWireAddress
    {
        std::array<uint8_t, 16> ip_bytes{};             // renamed from 'ip' for clarity
        std::uint16_t           port{0};                // stored in host byte order after conversion
        AddressFamily           family{AddressFamily::Unknown};
        std::uint8_t            padding{0};             // explicit — keeps size deterministic
    };

    static_assert(std::is_trivially_copyable_v<TickWireAddress>,
        "TickWireAddress must be trivially copyable");

    // Convert from OS sockaddr storage into TickWireAddress.
    // Returns false if addr is null, addr_len is too small, or family is unsupported.
    [[nodiscard]] bool from_sockaddr(const void* addr, int addr_len, TickWireAddress& out) noexcept;

    // Convert TickWireAddress back into OS sockaddr form (for send).
    // addr_len must be set to the size of the buffer before calling;
    // it is updated to the actual written size on success.
    [[nodiscard]] bool to_sockaddr(const TickWireAddress& in, void* addr, int& addr_len) noexcept;

} // namespace tickwire