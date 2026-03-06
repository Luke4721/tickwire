#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

#include "tickwire/packet.h"

namespace tickwire {

    constexpr std::size_t MAX_PAYLOAD = 1400;

    struct PacketBuffer {
        PacketMetadata metadata{};
        std::array<std::uint8_t, MAX_PAYLOAD> payload{};
    };

    static_assert(std::is_trivially_copyable_v<PacketBuffer>,
                  "PacketBuffer must be trivially copyable.");

} // namespace tickwire