#pragma once

#include <cstdint>
#include <type_traits>
#include "tickwire/address.h"

namespace tickwire
{
    // Transport-level metadata that precedes every payload.
    struct PacketMetadata
    {
        // Monotonic timestamp taken immediately after recvfrom()
        std::uint64_t   timestamp_ns{0};

        // Who sent the packet.
        TickWireAddress source{};

        // Size of payload in bytes
        std::uint16_t   payload_size{0};

        // Explicit padding — keeps alignment clean and avoids false sharing later.
        std::uint16_t   reserved{0};
    };

    // FIX: removed the outer parentheses that caused the comma-operator bug.
    // Previously: static_assert((condition, "message")) — always true.
    // Now:        static_assert(condition, "message")   — actually checks.
    static_assert(std::is_trivially_copyable_v<PacketMetadata>,
        "PacketMetadata must be trivially copyable.");

} // namespace tickwire