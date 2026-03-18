#pragma once

#ifdef _WIN32
#   include <winsock2.h>
#   include <ws2tcpip.h>
#else
#   include <sys/socket.h>
#endif

#include "tickwire/address.h"
#include "tickwire/socket.hpp"
#include "tickwire/buffer_pool.h"
#include "tickwire/spsc_queue.h"
#include "tickwire/packet_buffer.h"
#include "tickwire/metrics.h"

#include <cstdint>
#include <cstddef>

namespace tickwire
{
    class NetworkReceiver
    {
    public:
        NetworkReceiver(Socket&                   socket,
                        BufferPool&               pool,
                        SPSCQueue<PacketBuffer*>& queue,
                        Metrics&                  metrics) noexcept;

        // Receive one packet from the socket and push it into the queue.
        // Non-blocking: returns immediately if no data is available.
        void poll() noexcept;

        // Drain up to max_packets in a single call.
        // More efficient than calling poll() in a manual loop —
        // keeps the socket hot across iterations.
        // Returns the number of packets successfully enqueued.
        int drain(int max_packets) noexcept;

    private:
        Socket&                   socket_;
        BufferPool&               pool_;
        SPSCQueue<PacketBuffer*>& queue_;
        Metrics&                  metrics_;
    };

} // namespace tickwire