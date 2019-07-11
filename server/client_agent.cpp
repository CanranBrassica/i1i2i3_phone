#include "client_agent.hpp"
#include "chat_room.hpp"
#include "gateway.hpp"

#include <message.hpp>

#include <boost/algorithm/string.hpp>

namespace IpPhone
{


ClientAgent::ClientAgent(asio::io_context& io_context, GateWay& gateway)
    : socket{io_context}, gateway{gateway}
{

    using namespace std::string_literals;

    add_callback<Message::ExitMessage>([this](const Message::ExitMessage&) {
        exit();
    });

    add_callback<Message::CreateRoom>([this](const Message::CreateRoom& msg) {
        if (this->gateway.create_room(msg.room_id)) {
            std::cout << "create new room" << std::endl;
            async_send(Message::TextMessage{.talker_id = 0,
                .data = "create new room. room id: "s + std::to_string(msg.room_id)});
        } else {
            std::cout << "failed to create new room" << std::endl;
            async_send(Message::TextMessage{.talker_id = 0,
                .data = "failed to create new room."});
        }
    });

    add_callback<Message::JoinRoom>([this](const Message::JoinRoom& msg) {
        if (this->join_room(msg.room_id)) {
            async_send(Message::TextMessage{.talker_id = 0,
                .data = "join room("s + std::to_string(msg.room_id) + ")"});
        } else {
            async_send(Message::TextMessage{.talker_id = 0,
                .data = "failed to join room."});
        }
    });

    add_callback<Message::TextMessage>([this](const Message::TextMessage& msg) {
        if (!joinning_room.expired()) {
            joinning_room.lock()->tcp_multicast(msg,
                [](const std::shared_ptr<ClientAgent>& client, const boost::system::error_code& error, size_t len) {
                    if (error) {
                        std::cerr << "broadcast failed: " << error.message() << std::endl;
                    }
                });
        } else {
            std::cerr << "not join room" << std::endl;
        }
    });

    add_callback<Message::LeaveRoom>([this](const Message::LeaveRoom&) {
        joinning_room.lock()->leave(shared_from_this());
        async_send(Message::TextMessage{.talker_id = 0, .data = "leave room"});
    });
}

void ClientAgent::start_receive()
{
    if (!is_valid()) {
        return;  // invalidであればreadせずに無視
    }

    asio::async_read_until(
        socket, recv_buf, Message::END_OF_MESSAGE,  // END_OF_MESSAGEまで受け取る
        [self = shared_from_this()](auto&&... args) { self->on_receive(std::forward<decltype(args)>(args)...); });
}

void ClientAgent::on_receive(const boost::system::error_code& error, const size_t length)
{
    if (!is_valid()) {
        return;  // invalidであれば処理せずに無視
    }

    if (error == boost::asio::error::eof) {
        start_receive();
    } else if (error) {
        throw std::runtime_error{"client receive failed!! error: " + error.message()};
    }

    //    {
    //        const std::string str{asio::buffer_cast<const char*>(recv_buf.data()), recv_buf.size()};
    //        std::cout << "{";
    //        for (char c : str) {
    //            std::cout << +c << ", ";
    //        }
    //        std::cout << "}" << std::endl;
    //    }

    std::istream is{&recv_buf};
    Message::MessageIdType id;
    cereal::BinaryInputArchive ar{is};
    ar(id);
    assert(receive_callback[id]);
    receive_callback[id]();
    recv_buf.consume(1);  // for END_OF_MESSAGE

    start_receive();
}

void ClientAgent::close()
{
    id = std::nullopt;

    if (!joinning_room.expired()) {
        joinning_room.lock()->leave(shared_from_this());
        joinning_room.reset();
    }

    socket.close();
}

void ClientAgent::exit()
{
    gateway.remove_client(shared_from_this());
}

bool ClientAgent::join_room(size_t room_id)
{
    if (gateway.room_list[room_id]) {
        gateway.room_list[room_id]->join(shared_from_this());
        return true;
    }
    return false;
}

}  // namespace IpPhone
