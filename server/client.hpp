#pragma once

#include <boost/asio.hpp>

#include <iostream>

namespace IpPhone
{

namespace asio = boost::asio;

// 1clientにつき1オブジェクト作る．窓口
class ClientAgent : public std::enable_shared_from_this<ClientAgent>
{
    using tcp = asio::ip::tcp;

public:
    explicit ClientAgent(asio::io_context& io_context)
        : socket{io_context} {}

    void start_receive()
    {
        if (!is_valid()) {
            throw std::runtime_error{"This is invalid client!!"};
        }

        asio::async_read(
            socket,
            recv_buf,
            asio::transfer_at_least(1),  // 1byte以上受け取る
            [self = this->shared_from_this()](auto&&... args) { self->on_receive(std::forward<decltype(args)>(args)...); });
    }

    void on_receive(const boost::system::error_code& error, [[maybe_unused]] size_t length)
    {
        if (error == boost::asio::error::eof || length == 0) {
            start_receive();
        } else if (error) {
            throw std::runtime_error{"client receive failed!! error: " + error.message()};
        }

        std::string msg;
        {
            std::istream is(&recv_buf);
            is >> msg;
        }
        process_message(std::move(msg));
        start_receive();
    }

    void process_message(std::string&& message)
    {
        if (message == "\0") {
            return;
        }

        std::cout << "recv message: " << message << std::endl;
        if (message[0] == '/') {
            // execute command
        } else {

        }
    }

    bool is_valid() const
    {
        return id.has_value();
    }

    tcp::socket socket;
    std::optional<size_t> id = std::nullopt;

private:
    asio::streambuf recv_buf;
    std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> command_list;
};

}  // namespace IpPhone
