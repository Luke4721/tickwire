#include "tickwire/buffer_pool.h"
#include "tickwire/spsc_queue.h"
#include "tickwire/packet_buffer.h"

#include <chrono>
#include <iostream>

int main()
{
    using namespace tickwire;

    constexpr size_t NUM_PACKETS = 1'000'000;

    BufferPool pool(1024);
    SPSCQueue<PacketBuffer*> queue(1024);

    size_t dropped = 0;
    size_t pushed = 0;

    auto start = std::chrono::steady_clock::now();

    // 🔥 Simulate producer
    for (size_t i = 0; i < NUM_PACKETS; ++i)
    {
        auto buffer_opt = pool.acquire();
        if (!buffer_opt)
        {
            dropped++;
            continue;
        }

        PacketBuffer* buffer = *buffer_opt;

        if (!queue.push(buffer))
        {
            pool.release(buffer);
            dropped++;
            continue;
        }

        pushed++;
    }

    // 🔥 Simulate consumer
    size_t processed = 0;
    PacketBuffer* buffer = nullptr;

    while (queue.pop(buffer))
    {
        pool.release(buffer);
        processed++;
    }

    auto end = std::chrono::steady_clock::now();

    double seconds =
        std::chrono::duration<double>(end - start).count();

    std::cout << "=== TickWire Benchmark ===\n";
    std::cout << "Total attempted: " << NUM_PACKETS << "\n";
    std::cout << "Pushed: " << pushed << "\n";
    std::cout << "Processed: " << processed << "\n";
    std::cout << "Dropped: " << dropped << "\n";
    std::cout << "Time: " << seconds << " sec\n";
    std::cout << "Throughput: "
              << (processed / seconds)
              << " packets/sec\n";
}