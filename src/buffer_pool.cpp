#include "tickwire/buffer_pool.h"

namespace tickwire {

    BufferPool::BufferPool(std::size_t capacity) {
        buffers_.resize(capacity);
        free_list_.reserve(capacity);
        for (auto &buf : buffers_) {
            free_list_.push_back(&buf);
        }
    }

    std::optional<PacketBuffer*> BufferPool::acquire() {
        if (free_list_.empty()) {
            return std::nullopt;
        }
        auto* buf = free_list_.back();
        free_list_.pop_back();
        return buf;
    }

    void BufferPool::release(PacketBuffer* buffer) {
        free_list_.push_back(buffer);
    }

    std::size_t BufferPool::capacity() const noexcept {
        return buffers_.size();
    }

    std::size_t BufferPool::in_use() const noexcept {
        return buffers_.size() - free_list_.size();
    }

} // namespace tickwire