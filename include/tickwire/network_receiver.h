#pragma once

#include "tickwire/socket.hpp"
#include "tickwire/buffer_pool.h"
#include "tickwire/spsc_queue.h"
#include "tickwire/packet_buffer.h"
#include "tickwire/metrics.h"


namespace tickwire
{
    class NetworkReceiver
    {
        public:
            NetworkReceiver(Socket& socket,
            BufferPool& pool,
            SPSCQueue<PacketBuffer*>& queue,
            Metrics& metrics);

            void poll();
    private:
        Socket& socket_;
        BufferPool& pool_;
        SPSCQueue<PacketBuffer*>& queue_;
        Metrics& metrics_;

    };
}
