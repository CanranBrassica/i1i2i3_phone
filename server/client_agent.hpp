#pragma once

#include <boost/asio.hpp>
#include <iostream>

namespace IpPhone
{

class ChatRoom;
class GateWay;

namespace asio = boost::asio;

// 1clientにつき1オブジェクト作る．窓口
class ClientAgent : public std::enable_shared_from_this<ClientAgent>
{
    using tcp = asio::ip::tcp;

public:
    explicit ClientAgent(asio::io_context& io_context, std::shared_ptr<GateWay> gateway);

    void start_receive();
    void process_message(const std::string&& message);
    bool is_valid() const { return id.has_value(); }
    void close();

    tcp::socket socket;
    std::shared_ptr<GateWay> gateway;
    std::optional<size_t> id = std::nullopt;
    std::shared_ptr<ChatRoom> joinning_room = nullptr;

private:
    asio::streambuf recv_buf;
    std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> command_list;

    void on_receive(const boost::system::error_code& error, [[maybe_unused]] size_t length);
};

}  // namespace IpPhone
