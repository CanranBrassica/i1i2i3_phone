#include "client_agent.hpp"
#include "chat_room.hpp"
#include "gateway.hpp"

#include <boost/algorithm/string.hpp>

namespace IpPhone
{


ClientAgent::ClientAgent(asio::io_context& io_context, std::shared_ptr<GateWay> gateway)
    : socket{io_context}, gateway{std::move(gateway)}
{
    command_list["exit"] = [this]([[maybe_unused]] const std::vector<std::string>& argv) mutable {
        this->gateway->remove_client(shared_from_this());
    };

    command_list["create_room"] = [this](const std::vector<std::string>& argv) mutable {
        if (argv.size() >= 2) {
            this->gateway->create_room(std::strtoul(argv.at(1).c_str(), nullptr, 0));
        }
    };

    command_list["join_room"] = [this](const std::vector<std::string>& argv) mutable {
        auto id = std::strtoul(argv.at(1).c_str(), nullptr, 0);
        if (argv.size() >= 2 && this->gateway->room_list[id]) {
            std::cout << "join room_id: " << id << " client_id: " << this->id.value() << std::endl;
            this->gateway->room_list[id]->join(shared_from_this());
        }
    };
}

void ClientAgent::start_receive()
{
    if (!is_valid()) {
        return;  // invalidであればreadせずに無視
    }

    asio::async_read_until(
        socket, recv_buf, '\0',  // '\0'まで受け取る
        [self = this->shared_from_this()](auto&&... args) { self->on_receive(std::forward<decltype(args)>(args)...); });
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

    // @todo std::string以外にも対応させる
    const std::string msg = asio::buffer_cast<const char*>(recv_buf.data());
    recv_buf.consume(length);

    if (length > 0) {
        process_message(std::move(msg));
    }

    start_receive();
}

void ClientAgent::process_message(const std::string&& message)
{
    if (message == "\0") {
        return;
    }

    if (message[0] == '/') {
        std::vector<std::string> argv;
        boost::algorithm::split(argv, message, boost::is_any_of(" ,"));  // 空白文字かカンマで区切る
        boost::algorithm::trim_left_if(argv[0], boost::is_any_of("/"));  // コマンドの"/"を削除

        if (auto command_itr = command_list.find(argv[0]); command_itr != command_list.end()) {
            command_itr->second(argv);
        } else {
            std::cout << "invalid command: " << message << std::endl;
        }
    } else {
        if (joinning_room) {
            joinning_room->broadcast(std::move(message),
                [](const std::shared_ptr<ClientAgent>& client, const boost::system::error_code& error, size_t len) {
                    if (error) {
                        std::cerr << "broadcast failed: " << error.message() << std::endl;
                    }
                });
        } else {
            std::cerr << "not join room" << std::endl;
        }
    }
}

void ClientAgent::close()
{
    id = std::nullopt;
    gateway.reset();

    if (joinning_room) {
        joinning_room->leave(shared_from_this());
        joinning_room.reset();
    }

    socket.close();
}

}  // namespace IpPhone
