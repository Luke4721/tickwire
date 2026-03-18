#include "tickwire/network_receiver.h"
#include <chrono>
#include <cstring>

namespace tickwire {

    NetworkReceiver::NetworkReceiver(Socket&                   socket,
                                     BufferPool&               pool,
                                     SPSCQueue<PacketBuffer*>& queue,
                                     Metrics&                  metrics) noexcept
        : socket_(socket), pool_(pool), queue_(queue), metrics_(metrics) {}

    void NetworkReceiver::poll() noexcept
    {
        uint8_t temp_buffer[MAX_PAYLOAD];

        sockaddr_storage storage{};
        int addr_len = static_cast<int>(sizeof(storage));

        int received = socket_.receive(temp_buffer, MAX_PAYLOAD, &storage, &addr_len);

        if (received <= 0)
            return;

        TickWireAddress sender{};
        if (!from_sockaddr(&storage, addr_len, sender))
            return; // unknown address family — discard packet

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
        buffer->metadata.source       = sender;

        const auto now = std::chrono::steady_clock::now().time_since_epoch();
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

    int NetworkReceiver::drain(int max_packets) noexcept
    {
        int count = 0;
        while (count < max_packets)
        {
            uint8_t temp_buffer[MAX_PAYLOAD];

            sockaddr_storage storage{};
            int addr_len = static_cast<int>(sizeof(storage));

            int received = socket_.receive(temp_buffer, MAX_PAYLOAD, &storage, &addr_len);

            if (received <= 0)
                break; // no more data available right now

            TickWireAddress sender{};
            if (!from_sockaddr(&storage, addr_len, sender))
                continue; // bad address, skip this packet

            ++metrics_.packets_received;

            auto buffer_opt = pool_.acquire();
            if (!buffer_opt)
            {
                ++metrics_.packets_dropped_pool;
                continue;
            }

            PacketBuffer* buffer = *buffer_opt;

            std::memcpy(buffer->payload.data(), temp_buffer, received);
            buffer->metadata.payload_size = static_cast<std::uint16_t>(received);
            buffer->metadata.source       = sender;

            const auto now = std::chrono::steady_clock::now().time_since_epoch();
            buffer->metadata.timestamp_ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

            ++metrics_.packets_processed;

            if (!queue_.push(buffer))
            {
                ++metrics_.packets_dropped_queue;
                pool_.release(buffer);
                continue;
            }

            ++metrics_.packets_enqueued;
            ++count;
        }

        return count;
    }

} // namespace tickwire