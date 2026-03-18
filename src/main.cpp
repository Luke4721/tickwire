#include "tickwire/socket.hpp"
#include "tickwire/buffer_pool.h"
#include "tickwire/spsc_queue.h"
#include "tickwire/network_receiver.h"
#include "tickwire/worker.h"
#include "tickwire/metrics.h"

#include <iostream>

int main()
{
    using namespace tickwire;

    Socket socket;

    // FIX: open_udp was called TWICE before.
    // First call succeeded and bound port 54000 — return value was ignored.
    // Second call tried to bind the same port again → WSAEADDRINUSE → returned false.
    // Result: the program always printed "Failed to open socket" and exited immediately.
    if (!socket.open_udp(54000))
    {
        std::cerr << "Failed to open socket on port 54000\n";
        return -1;
    }

    std::cout << "TickWire listening on UDP port 54000...\n";

    BufferPool               pool(1024);
    SPSCQueue<PacketBuffer*> queue(1024);
    Metrics                  metrics;

    NetworkReceiver receiver(socket, pool, queue, metrics);
    Worker          worker(queue, pool);

    int counter = 0;

    while (true)
    {
        // drain() is more efficient than poll() in a tight loop:
        // it keeps the socket hot across up to 64 back-to-back receives
        // before yielding to the worker.
        receiver.drain(64);
        worker.process();

        ++counter;

        if (counter % 128 == 0)
        {
            std::cout << "\n==== TickWire Stats ====\n";
            std::cout << "Received:       " << metrics.packets_received.load()     << "\n";
            std::cout << "Enqueued:       " << metrics.packets_enqueued.load()     << "\n";
            std::cout << "Dropped (pool): " << metrics.packets_dropped_pool.load() << "\n";
            std::cout << "Dropped (queue):" << metrics.packets_dropped_queue.load()<< "\n";
            std::cout << "========================\n";
        }
    }

    return 0;
}