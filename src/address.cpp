// src/address.cpp
#include "tickwire/address.h"

#ifdef _WIN32
#   include <winsock2.h>
#   include <ws2tcpip.h>
#else
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#endif

#include <cstring>

namespace tickwire {

    bool from_sockaddr(const void* addr, int addr_len, TickWireAddress& out) noexcept
    {
        if (!addr || addr_len <= 0)
            return false;

        const auto* sa = static_cast<const sockaddr*>(addr);

        if (sa->sa_family == AF_INET)
        {
            if (addr_len < static_cast<int>(sizeof(sockaddr_in)))
                return false;

            const auto* s4 = static_cast<const sockaddr_in*>(addr);
            out.family = AddressFamily::IPv4;
            out.port   = ntohs(s4->sin_port);  // convert to host byte order for consistency
            std::memcpy(out.ip_bytes.data(), &s4->sin_addr, 4);
            return true;
        }

        if (sa->sa_family == AF_INET6)
        {
            if (addr_len < static_cast<int>(sizeof(sockaddr_in6)))
                return false;

            const auto* s6 = static_cast<const sockaddr_in6*>(addr);
            out.family = AddressFamily::IPv6;
            out.port   = ntohs(s6->sin6_port);
            std::memcpy(out.ip_bytes.data(), &s6->sin6_addr, 16);
            return true;
        }

        return false; // unsupported family
    }

    bool to_sockaddr(const TickWireAddress& in, void* addr, int& addr_len) noexcept
    {
        if (!addr)
            return false;

        if (in.family == AddressFamily::IPv4)
        {
            if (addr_len < static_cast<int>(sizeof(sockaddr_in)))
                return false;

            auto* s4 = static_cast<sockaddr_in*>(addr);
            std::memset(s4, 0, sizeof(sockaddr_in));
            s4->sin_family = AF_INET;
            s4->sin_port   = htons(in.port);
            std::memcpy(&s4->sin_addr, in.ip_bytes.data(), 4);
            addr_len = static_cast<int>(sizeof(sockaddr_in));
            return true;
        }

        if (in.family == AddressFamily::IPv6)
        {
            if (addr_len < static_cast<int>(sizeof(sockaddr_in6)))
                return false;

            auto* s6 = static_cast<sockaddr_in6*>(addr);
            std::memset(s6, 0, sizeof(sockaddr_in6));
            s6->sin6_family = AF_INET6;
            s6->sin6_port   = htons(in.port);
            std::memcpy(&s6->sin6_addr, in.ip_bytes.data(), 16);
            addr_len = static_cast<int>(sizeof(sockaddr_in6));
            return true;
        }

        return false; // Unknown family
    }

} // namespace tickwire