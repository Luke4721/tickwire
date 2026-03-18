#pragma once

#include <vector>
#include <atomic>
#include <cstddef>

namespace tickwire
{
    template<typename T>
    class SPSCQueue
    {
    public:
        explicit SPSCQueue(std::size_t capacity)
            : buffer_(capacity),
              capacity_(capacity)
        {
            // head_ and tail_ are zero-initialized by their declarations.
        }

        // Called by the PRODUCER thread only.
        bool push(const T& item) noexcept
        {
            const auto head = head_.load(std::memory_order_relaxed);
            const auto next = increment(head);

            if (next == tail_.load(std::memory_order_acquire))
                return false; // queue full

            buffer_[head] = item;
            head_.store(next, std::memory_order_release);
            return true;
        }

        // Called by the CONSUMER thread only.
        bool pop(T& item) noexcept
        {
            const auto tail = tail_.load(std::memory_order_relaxed);

            if (tail == head_.load(std::memory_order_acquire))
                return false; // queue empty

            item = buffer_[tail];
            tail_.store(increment(tail), std::memory_order_release);
            return true;
        }

    private:
        std::size_t increment(std::size_t index) const noexcept
        {
            return (index + 1) % capacity_;
        }

    private:
        std::vector<T>  buffer_;
        std::size_t     capacity_;

        // FIX: false sharing.
        // head_ is written by the producer; tail_ is written by the consumer.
        // If they share a cache line, every write by one core invalidates
        // the cache line on the other, causing a cache miss on every iteration.
        // alignas(64) places each atomic on its own 64-byte cache line,
        // eliminating the cross-core invalidation entirely.
        static constexpr std::size_t CACHE_LINE_SIZE = 64;

        alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> head_{0};
        alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> tail_{0};
    };

} // namespace tickwire