#include "tickwire/socket.hpp"

namespace tickwire
{
    Socket::Socket() noexcept = default;

    Socket::~Socket()
    {
        close();
    }

    Socket::Socket(Socket&& other) noexcept
    {
        handle_       = other.handle_;
        other.handle_ = INVALID_HANDLE;
    }

    Socket& Socket::operator=(Socket&& other) noexcept
    {
        if (this != &other)
        {
            close();
            handle_       = other.handle_;
            other.handle_ = INVALID_HANDLE;
        }
        return *this;
    }

    bool Socket::is_valid() const noexcept
    {
        return handle_ != INVALID_HANDLE;
    }

} // namespace tickwire