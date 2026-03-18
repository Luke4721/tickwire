#include "tickwire/worker.h"

#include <iostream>

namespace tickwire
{
    Worker::Worker(SPSCQueue<PacketBuffer*>& queue,
        BufferPool& pool)
            : queue_(queue), pool_(pool) {}

    void Worker::process() {
        PacketBuffer* buffer{nullptr};
        while (queue_.pop(buffer)) {
            // TODO: replace with real application logic
            // e.g. parse payload, update state, forward to another system
            pool_.release(buffer);
        }
    }
}
