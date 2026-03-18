#pragma once

#include <atomic>
#include <cstdint>

namespace tickwire
{
    struct Metrics
    {
        std::atomic<uint32_t> packets_received{0};
        std::atomic<uint32_t> packets_enqueued{0};
        std::atomic<uint32_t> packets_dropped_pool{0};
        std::atomic<uint32_t> packets_dropped_queue{0};
        std::atomic<uint32_t> packets_processed{0};
    };
}