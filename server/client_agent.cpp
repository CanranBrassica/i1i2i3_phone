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
                []([[maybe_unused]] const std::shared_ptr<ClientAgent>& client, const boost::system::error_code& error, [[maybe_unused]] size_t len) {
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

    add_callback<Message::JoinMulticast>([this](const Message::JoinMulticast&) {
        if (joinning_room.expired()) {
            std::cerr << "not join room" << std::endl;
            return;
        }
        auto jr = joinning_room.lock();
        if (!jr->multicast_config.has_value()) {
            jr->multicast_config = this->gateway.pay_out_multicast_config();
        }

        async_send(Message::MulticastConfig{
            .multicast_address = std::get<0>(*jr->multicast_config).to_string(),
            .multicast_port = std::get<1>(*jr->multicast_config)});
    });

    add_callback<Message::PrintRoomListRequest>([this](auto) {
        std::stringstream ss;
        if (this->gateway.room_list.empty()) {
            ss << "no room";
        }
        for (const auto& room : this->gateway.room_list) {
            ss << "room_id: " << room.first << "\n";
            if (room.second->member.empty()) {
                ss << "\t no user";
            }
            for (const auto& mem : room.second->member) {
                ss << "\tuser_id: " << mem.lock()->id.value() << "\n";
            }
        }
        async_send(Message::TextMessage{.talker_id = 0, .data = ss.str()});
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

void ClientAgent::on_receive(const boost::system::error_code& error, [[maybe_unused]] const size_t length)
{
    if (!is_valid()) {
        return;  // invalidであれば処理せずに無視
    }

    if (error == boost::asio::error::eof) {
        start_receive();
    } else if (error) {
        throw std::runtime_error{"client receive failed!! error: " + error.message()};
    }

    std::istream is{&recv_buf};
    Message::MessageIdType id;
    cereal::BinaryInputArchive ar{is};
    ar(id);
    assert(receive_callback[id]);
    receive_callback[id]();
    recv_buf.consume(sizeof(Message::END_OF_MESSAGE));  // for END_OF_MESSAGE

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
