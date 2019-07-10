#pragma once

#include "client_agent.hpp"

#include <message.hpp>

#include <boost/asio.hpp>
#include <unordered_set>
#include <memory>


namespace IpPhone
{

namespace asio = boost::asio;

class ChatRoom : public std::enable_shared_from_this<ChatRoom>
{

public:
    ChatRoom() = default;

    void join(const std::shared_ptr<ClientAgent>& client)
    {
        client->joinning_room = shared_from_this();
        member.insert(client);
    }

    void leave(const std::shared_ptr<ClientAgent>& client)
    {
        client->joinning_room = nullptr;
        member.erase(client);
    }

    template <class TMessage, class F>
    void broadcast(TMessage&& msg, F&& on_send)
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
            asio::async_write(
                client->socket, asio::buffer(*data),
                [client, on_send = std::forward<F>(on_send), data](auto&&... args) {
                    on_send(client, std::forward<decltype(args)>(args)...);
                });
        }
    }

    std::unordered_set<std::shared_ptr<ClientAgent>> member;
};

}  // namespace IpPhone
