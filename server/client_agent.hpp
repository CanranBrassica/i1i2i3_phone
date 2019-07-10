#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>

#include <message.hpp>

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
    explicit ClientAgent(asio::io_context& io_context, GateWay& gateway);
    ~ClientAgent() { close(); }

    void start_receive();
    bool is_valid() const { return id.has_value(); }
    void close();

    template <class TMessage, class F>
    void add_callback(F&& func);

    template <class TMessage>
    void async_send(TMessage&& msg);

    tcp::socket socket;
    GateWay& gateway;
    std::optional<size_t> id = std::nullopt;
    std::weak_ptr<ChatRoom> joinning_room;

private:
    asio::streambuf recv_buf;
    void exit();
    bool join_room(size_t room_id);
    void broadcast_message(const Message::TextMessage& msg);

    void on_receive(const boost::system::error_code& error, [[maybe_unused]] size_t length);

    std::unordered_map<size_t, std::function<void()>> receive_callback;
};

template <class TMessage, class F>
void ClientAgent::add_callback(F&& func)
{
    if (!receive_callback[TMessage::MessageId]) {
        receive_callback[TMessage::MessageId] = [func = std::forward<F>(func), this] {
            TMessage ret;
            std::istream is{&recv_buf};
            cereal::BinaryInputArchive ar{is};
            ar(ret);
            func(std::move(ret));
        };
    } else {
        std::cout << "This id is already in use." << std::endl;
    }
}

template <class TMessage>
void ClientAgent::async_send(TMessage&& msg)
{
    std::stringstream ss;
    {
        cereal::BinaryOutputArchive ar{ss};
        ar(TMessage::MessageId);
        ar(msg);
    }

    auto data = std::make_shared<std::string>(std::move(ss.str() + Message::END_OF_MESSAGE));  // dataの寿命をasync_writeが全て終わるまで伸ばす

    asio::async_write(
        socket, asio::buffer(*data),
        [](const boost::system::error_code& error, size_t len) {
            if (error) {
                std::cerr << "send failed: " << error.message() << std::endl;
            }
        });
}

}  // namespace IpPhone
