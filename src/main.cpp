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
    socket.open_udp(54000); // example port
    if (!socket.open_udp(54000))
    {
        std::cout << "Failed to open socket\n";
        return -1;
    }

    BufferPool pool(1024);
    SPSCQueue<PacketBuffer*> queue(1024);
    Metrics metrics;

    NetworkReceiver receiver(socket, pool, queue, metrics);
    Worker worker(queue, pool);

    int counter = 0;

    while (true)
    {
        receiver.poll();     // network side
        worker.process();    // worker side

        counter++;

        if (counter % 128 == 0)
        {
            std::cout << "\n==== TickWire Stats ====\n";
            std::cout << "Received: " << metrics.packets_received.load() << "\n";
            std::cout << "Enqueued: " << metrics.packets_enqueued.load() << "\n";
            std::cout << "Dropped (pool): " << metrics.packets_dropped_pool.load() << "\n";
            std::cout << "Dropped (queue): " << metrics.packets_dropped_queue.load() << "\n";
            std::cout << "========================\n";
        }
    }

    return 0;
}