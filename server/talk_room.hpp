#pragma once

#include <vector>
#include <memory>

#include <boost/asio.hpp>

namespace IpPhone
{
class Client
{
public:
    virtual ~Client() {}
};

class TalkRoom
{
    using udp = boost::asio::ip::udp;

public:
private:
    std::vector<std::shared_ptr<Client>> talkers;
};
}  // namespace IpPhone
