#include "tickwire/worker.h"

#include <iostream>

namespace tickwire
{
    Worker::Worker(SPSCQueue<PacketBuffer*>& queue,
        BufferPool& pool)
            : queue_(queue), pool_(pool) {}

    void Worker::process()
    {
        PacketBuffer* buffer{nullptr};

        while (queue_.pop(buffer))
        {
            //Simulate processing
            const auto& meta {buffer->metadata};
            std:: cout<<"Received Packet : "<<meta.payload_size<< " bytes\n";

            //Later we can:
            // - parse Payload
            // - game logic
            // - AI logic etc.

            //Returning the buffer to the pool
            pool_.release(buffer);
        }

    }
}
