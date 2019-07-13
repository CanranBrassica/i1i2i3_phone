#pragma once

#include <message.hpp>

#include <boost/asio.hpp>
#include <cereal/archives/binary.hpp>
#include <iostream>

namespace IpPhone
{
struct TcpConnection
{
    using tcp = boost::asio::ip::tcp;

    TcpConnection(boost::asio::ip::tcp::socket& socket,
                  const boost::asio::ip::address_v4& server_address,
                  unsigned short server_port)
        : socket{socket}
    {
        socket.connect(tcp::endpoint(server_address, server_port));
    }


    template <class T>
    void send(const T& msg)
    {
        using TMessage = std::decay_t<T>;

        std::stringstream ss;
        {
            cereal::BinaryOutputArchive ar{ss};
            ar(TMessage::MessageId);
            ar(msg);
        }

        auto data = std::make_shared<std::string>(ss.str() + Message::END_OF_MESSAGE);

        boost::asio::async_write(
            socket,
            boost::asio::buffer(*data),
            [data](const boost::system::error_code& error, [[maybe_unused]] size_t length) {
                if (error) {
                    std::cerr << "send failed: " << error.message() << std::endl;
                }
            });
    }

    void start_receive()
    {
        namespace asio = boost::asio;
        asio::async_read_until(
            socket, recv_buf, IpPhone::Message::END_OF_MESSAGE,
            [this](const boost::system::error_code& error, [[maybe_unused]] size_t length) mutable {
                if (error && error != boost::asio::error::eof) {
                    std::cout << "receive failed: " << error.message() << std::endl;
                }

                receive_message();
                start_receive();
            });
    }

    void receive_message()
    {
        std::istream is{&recv_buf};
        IpPhone::Message::MessageIdType id;
        cereal::BinaryInputArchive ar{is};
        ar(id);
        assert(receive_callback[id]);
        receive_callback[id]();
        recv_buf.consume(1);  // for END_OF_MESSAGE
    }

    template <class TMessage, class F>
    int add_callback(F&& func)
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

        return 0;
    }

    boost::asio::ip::tcp::socket& socket;
    std::unordered_map<size_t, std::function<void()>> receive_callback;
    boost::asio::streambuf recv_buf;
};
}  // namespace IpPhone
