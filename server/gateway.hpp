#pragma once

#include "client_agent.hpp"
#include "chat_room.hpp"

#include <util/id_registrator.hpp>

#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace IpPhone
{

namespace asio = boost::asio;

class GateWay : public std::enable_shared_from_this<GateWay>
{
    using tcp = asio::ip::tcp;

public:
    GateWay(asio::io_context& io_context, unsigned short port)
        : acceptor{io_context, tcp::endpoint(tcp::v4(), port)},
          io_context{io_context} {}

    void start_accept()
    {
        client_list.emplace_back(std::make_shared<ClientAgent>(io_context, *this));
        acceptor.async_accept(client_list.back()->socket,
            [self = shared_from_this()](auto&&... args) {
                self->on_accept(std::forward<decltype(args)>(args)...);
            });
    }

    void on_accept(const boost::system::error_code& error)
    {
        if (error) {
            throw std::runtime_error{"accept failed!! error: " + error.message()};
        } else {
            std::cout << "accept! id: " << client_id_registrator.peekNewId() << std::endl;
        }
        client_list.back()->id = client_id_registrator.registerNewId();
        client_list.back()->start_receive();
        client_list.back()->async_send(Message::SetUserId{.user_id = client_list.back()->id.value()});

        start_accept();  // 次の接続要求を待つ
    }

    void remove_client(const std::shared_ptr<ClientAgent>& client)
    {
        if (auto itr = std::find(client_list.begin(), client_list.end(), client);
            itr != client_list.end()) {
            std::cerr << "remove client. id: " << client->id.value() << std::endl;
            (*itr)->close();
            client_list.erase(itr);
        }
    }

    bool create_room(size_t id)
    {
        if (!room_list[id]) {
            room_list[id] = std::make_shared<ChatRoom>(io_context);
            std::cout << "new chat room. id: " << id << std::endl;
            return true;
        }
        return false;
    }


private:
    friend ClientAgent;

    tcp::acceptor acceptor;
    asio::io_context& io_context;
    std::vector<std::shared_ptr<ClientAgent>> client_list;
    std::unordered_map<size_t, std::shared_ptr<ChatRoom>> room_list;

    IdRegistrator<size_t> client_id_registrator = {1};  // 0はserver用
    IdRegistrator<unsigned short> room_id_registrator = {4000};
};
}  // namespace IpPhone
