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
            :buffer_(capacity),
        capacity_(capacity),
        head_(0),
        tail_(0){}

        bool push(const T& item)
        {
            auto head = head_.load(std::memory_order_relaxed);
            auto next = increment(head);

            if (next == tail_.load(std::memory_order_acquire)) {
                return false; // queue full
            }

            buffer_[head] = item;
            head_.store(next, std::memory_order_release);
            return true;
        }
        bool pop(T& item) {
            auto tail = tail_.load(std::memory_order_relaxed);

            if (tail == head_.load(std::memory_order_acquire)) {
                return false; // queue empty
            }

            item = buffer_[tail];
            tail_.store(increment(tail), std::memory_order_release);
            return true;
        }

    private:
        std::size_t increment(std::size_t index) const {
            return (index + 1) % capacity_;
        }

    private:
        std::vector<T> buffer_;
        std::size_t capacity_;

        std::atomic<std::size_t> head_;
        std::atomic<std::size_t> tail_;
    };
}
