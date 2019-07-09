#pragma once

#include <boost/asio.hpp>

namespace IpPhone
{

namespace asio = boost::asio;

class Voice
{
    using udp = asio::ip::udp;

public:
    Voice(asio::io_context& io_context, unsigned short port)
        : socket(io_context, udp::endpoint(udp::v4(), port)) {}

private:
    udp::socket socket;
};
}  // namespace IpPhone
