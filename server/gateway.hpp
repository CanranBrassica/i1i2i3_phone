#pragma once

#include "client.hpp"
#include "chat_room.hpp"

#include <util/id_registrator.hpp>

#include <boost/asio.hpp>
#include <memory>
#include <functional>

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
        client_list.emplace_back(std::make_shared<ClientAgent>(io_context));
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
            std::cout << "accept!!" << std::endl;
        }
        client_list.back()->id = id_registrator.registerNewId();
        client_list.back()->start_receive();

        start_accept();  // 次の接続要求を待つ
    }

private:
    tcp::acceptor acceptor;
    asio::io_context& io_context;
    std::vector<std::shared_ptr<ClientAgent>> client_list;

    IdRegistrator<size_t> id_registrator;
};
}  // namespace IpPhone
