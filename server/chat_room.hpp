#pragma once

#include "client.hpp"

#include <unordered_set>

namespace IpPhone
{

class ChatRoom
{
    ChatRoom() = default;

    void join(const std::shared_ptr<ClientAgent>& client)
    {
        member.insert(client);
    }

    void leave(const std::shared_ptr<ClientAgent>& client)
    {
        member.erase(client);
    }

    template <class TData, class F>
    void broadcast(TData&& data_, F&& on_broadcast)
    {
        auto data = std::make_shared<std::remove_reference_t<TData>>(std::forward<TData>(data_));  // dataの寿命をasync_writeが全て終わるまで伸ばす
        for (auto& client : member) {
            asio::async_write(
                client->socket, *data,
                [client, on_broadcast = std::forward<F>(on_broadcast), data](auto&&... args) {
                    on_broadcast(client, std::forward<decltype(args)>(args)...);
                });
        }
    }

    std::unordered_set<std::shared_ptr<ClientAgent>> member;
};

}  // namespace IpPhone
