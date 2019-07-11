#pragma once

#include "client_agent.hpp"

#include <message.hpp>

#include <boost/asio.hpp>
#include <forward_list>
#include <memory>


namespace IpPhone
{

namespace asio = boost::asio;

class ChatRoom : public std::enable_shared_from_this<ChatRoom>
{

    using udp = boost::asio::ip::udp;

public:
    ChatRoom(asio::io_context& io_context) {}
    //    ChatRoom(asio::io_context& io_context, const udp::endpoint& udp_receive_endpoint)
    //        : receive_socket{io_context}
    //    {
    //    }

    void join(const std::shared_ptr<ClientAgent>& client)
    {
        client->joinning_room = shared_from_this();
        member.emplace_front(client);
    }

    void leave(const std::shared_ptr<ClientAgent>& client)
    {
        client->joinning_room.reset();
        member.remove_if([&client](const std::weak_ptr<ClientAgent>& c) {
            return !c.expired() || c.lock() == client;
        });
    }

    template <class TMessage, class F>
    void tcp_multicast(TMessage&& msg, F&& on_send)  // tcpを用いて擬似的にchatroom内にmulticast
    {
        std::stringstream ss;
        {
            cereal::BinaryOutputArchive ar{ss};
            ar(std::decay_t<TMessage>::MessageId);
            ar(msg);
        }

        auto data = std::make_shared<std::string>(std::move(ss.str() + Message::END_OF_MESSAGE));  // dataの寿命をasync_writeが全て終わるまで伸ばす

        // 各client毎にasync_sendをするとdataがn個複製されてしまう
        for (auto& client : member) {
            if (!client.expired()) {
                asio::async_write(
                    client.lock()->socket, asio::buffer(*data),
                    [client, on_send = std::forward<F>(on_send), data](auto&&... args) {
                        on_send(client.lock(), std::forward<decltype(args)>(args)...);
                    });
            }
        }
    }


    template <class TMessage, class F>
    void udp_multicast(TMessage&& msg, F&& on_send)
    {
        std::stringstream ss;
        {
            cereal::BinaryOutputArchive ar{ss};
            ar(std::decay_t<TMessage>::MessageId);
            ar(msg);
        }

        auto data = std::make_shared<std::string>(std::move(ss.str() + Message::END_OF_MESSAGE));  // dataの寿命をasync_writeが全て終わるまで伸ばす
    }


private:
    std::forward_list<std::weak_ptr<ClientAgent>> member;

    //    udp::socket multicast_socket;
    //    udp::endpoint multicast_endpoint;

    //    udp::socket receive_socket;
};

}  // namespace IpPhone
