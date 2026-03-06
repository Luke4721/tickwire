#include "tickwire/socket.hpp"

namespace tickwire
{
    Socket::Socket() = default;

    Socket::~Socket() { close();}

    Socket::Socket(Socket&& other) noexcept
    {
        handle_ = other.handle_;
        other.handle_ = -1;
    }

    Socket& Socket::operator=(Socket&& other) noexcept
    {
        if (this!=&other)
        {
            close();
            handle_ = other.handle_;
            other.handle_ = -1;
        }
        return *this;
    }

    bool Socket::is_valid() const noexcept
    {
        return handle_ != -1;
    }
}//namespace tickwire
