#include "tickwire/address.h"

#include <cstring>

#if defined (_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

#endif

namespace tickwire
{
    bool from_sockaddr(const void* addr, TickWireAddress& out)
    {
        const sockaddr* sa = reinterpret_cast<const sockaddr*>(addr);

        if (sa->sa_family == AF_INET) //If the pointer is IPv4
        {
            const sockaddr_in* v4 = reinterpret_cast<const sockaddr_in*>(sa);

            //Clear address first
            out.ip.fill(0);

            // IPv4-mapped IPv6 prefix ::ffff
            out.ip[10] = 0xff;
            out.ip[11] = 0xff;

            // Copy IPv4 bytes into last 4 bytes.
            std::memcpy(&out.ip[12], &v4->sin_addr,4);

            out.port = v4->sin_port;
            out.family = AF_INET;
            return true;
        }

        if (sa->sa_family == AF_INET6) {                    //  If the pointer is IPv6
            const sockaddr_in6* v6 = reinterpret_cast<const sockaddr_in6*>(sa);

            std::memcpy(out.ip.data(), &v6->sin6_addr, 16);

            out.port   = v6->sin6_port;
            out.family = AF_INET6;
            return true;
        }
        return false;
    }

    bool to_sockaddr(const TickWireAddress& in, void* addr, int& addr_len)
    {
        if (in.family == AF_INET) {
            sockaddr_in* v4 = reinterpret_cast<sockaddr_in*>(addr);
            std::memset(v4, 0, sizeof(sockaddr_in));

            v4->sin_family = AF_INET;
            v4->sin_port   = in.port;

            // Extract IPv4 from mapped form
            std::memcpy(&v4->sin_addr, &in.ip[12], 4);

            addr_len = sizeof(sockaddr_in);
            return true;
        }

        if (in.family == AF_INET6) {
            sockaddr_in6* v6 = reinterpret_cast<sockaddr_in6*>(addr);
            std::memset(v6, 0, sizeof(sockaddr_in6));

            v6->sin6_family = AF_INET6;
            v6->sin6_port   = in.port;

            std::memcpy(&v6->sin6_addr, in.ip.data(), 16);

            addr_len = sizeof(sockaddr_in6);
            return true;
        }

        return false;
    }

} // namespace tickwire


