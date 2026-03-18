#pragma once

#include "tickwire/spsc_queue.h"
#include "tickwire/buffer_pool.h"
#include "tickwire/packet_buffer.h"

namespace tickwire
{
    class Worker
    {
    public:
        Worker(SPSCQueue<PacketBuffer*>& queue,
            BufferPool& pool);

        void process();

    private:
        SPSCQueue<PacketBuffer*>& queue_;
        BufferPool& pool_;
    };
}
