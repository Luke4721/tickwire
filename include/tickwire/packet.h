#pragma once

#include <cstdint>
#include "tickwire/address.h"
#include <type_traits>

namespace tickwire
{
    // Transport-level metadata that precedes every payload.
    struct PacketMetadata
    {
        // Monotonic timestamp taken immediately after recvfrom()
        std::uint64_t timestamp_ns{0};

        // Who sent the packet.
        TickWireAddress source{};

        // Size of payload in bytes
        std::uint16_t payload_size{0};

        // Padding to keep alignment clean (to avoid false sharing later)
        std::uint16_t reserved{0};
    };
    static_assert((std::is_trivially_copyable_v<PacketMetadata>,
        "PacketMetadata must be trivially copyable."));
}