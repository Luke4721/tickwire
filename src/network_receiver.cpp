#include "tickwire/network_receiver.h"
#include <chrono>
#include <cstring>

namespace tickwire {

    NetworkReceiver::NetworkReceiver(Socket& socket,
                                     BufferPool& pool,
                                     SPSCQueue<PacketBuffer*>& queue,
                                     Metrics& metrics
                                     )
        : socket_(socket), pool_(pool), queue_(queue), metrics_(metrics) {}

    void NetworkReceiver::poll()
    {
        std::uint8_t temp_buffer[MAX_PAYLOAD];

        TickWireAddress sender{};
        std::size_t received = 0;

        if (!socket_.receive(temp_buffer, MAX_PAYLOAD, sender, received))
            return;
        ++metrics_.packets_received;

        auto buffer_opt = pool_.acquire();

        if (!buffer_opt)
        {
            ++metrics_.packets_dropped_pool;
            return;
        }

        PacketBuffer* buffer = *buffer_opt;

        std::memcpy(buffer->payload.data(), temp_buffer, received);

        buffer->metadata.payload_size = static_cast<std::uint16_t>(received);
        buffer->metadata.source = sender;

        auto now = std::chrono::steady_clock::now().time_since_epoch();
        buffer->metadata.timestamp_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
        ++metrics_.packets_processed;

        if (!queue_.push(buffer))
        {
            ++metrics_.packets_dropped_queue;
            pool_.release(buffer);
            return;
        }
        ++metrics_.packets_enqueued;
    }

}