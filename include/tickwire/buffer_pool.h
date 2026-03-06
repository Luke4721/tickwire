#pragma once

#include "tickwire/packet_buffer.h"
#include <vector>
#include <optional>

namespace tickwire {

    class BufferPool {
    public:
        BufferPool(std::size_t capacity);

        // Acquire the next free buffer, or nullopt if none available
        std::optional<PacketBuffer*> acquire();

        // Return a buffer to the pool
        void release(PacketBuffer* buffer);

        // Get current capacity and usage
        std::size_t capacity() const noexcept;
        std::size_t in_use() const noexcept;

    private:
        std::vector<PacketBuffer> buffers_;
        std::vector<PacketBuffer*> free_list_;
    };

} // namespace tickwire