#pragma once

#include <boost/asio.hpp>
#include <boost/interprocess/streams/vectorstream.hpp>

#include <sstream>

#include <cereal/archives/binary.hpp>

namespace IpPhone
{

struct UdpMulticastMessage : public std::enable_shared_from_this<UdpMulticastMessage>
{
    using udp = boost::asio::ip::udp;

    UdpMulticastMessage(boost::asio::io_context& io_context,
        const boost::asio::ip::address_v4& multicast_address,
        unsigned short multicast_port,
        const boost::asio::ip::address_v4& network_interface,
        const boost::asio::ip::address_v4& listen_address = boost::asio::ip::address_v4::any())
        : recv_socket{io_context}, multicast_endpoint{multicast_address, multicast_port},
          send_socket{io_context, multicast_endpoint.protocol()}
    {
        udp::endpoint listen_endpoint(listen_address, multicast_port);
        recv_socket.open(listen_endpoint.protocol());
        recv_socket.set_option(udp::socket::reuse_address(true));
        recv_socket.bind(listen_endpoint);
        recv_socket.set_option(boost::asio::ip::multicast::join_group(multicast_address));

        send_socket.set_option(boost::asio::ip::multicast::outbound_interface(network_interface));
        send_socket.set_option(boost::asio::ip::multicast::enable_loopback(true));

        //        start_receive();
    }

    void start_receive()
    {
        auto buf = std::make_shared<std::array<char, 8192>>();
        recv_socket.async_receive_from(
            boost::asio::buffer(*buf), sender_endpoint,
            [weak_self = std::weak_ptr{shared_from_this()}, buf](boost::system::error_code ec, std::size_t length) {
                if (!weak_self.expired()) {
                    std::lock_guard{weak_self.lock()->s_buf_mtx};
                    weak_self.lock()->s_buf.sputn(buf->data(), length);  // bufに書き込み
                    weak_self.lock()->on_recv(ec, length);               // 受信処理
                }
            });
    }

    void on_recv(boost::system::error_code ec, [[maybe_unused]] std::size_t length)
    {
        if (ec == boost::asio::error::eof) {
            start_receive();
        } else if (ec) {
            throw std::runtime_error{"client receive failed!! error: " + ec.message()};
        }


        {
            std::lock_guard{s_buf_mtx};
            while (s_buf.size() > 0) {
                Message::MessageIdType id;

                std::istream is{&s_buf};
                cereal::BinaryInputArchive ar{is};
                ar(id);

                assert(receive_callback[id]);
                receive_callback[id]();
            }
        }

        start_receive();
    }

    template <class TMessage, class F>
    void add_callback(F&& func)
    {
        if (!receive_callback[TMessage::MessageId]) {
            receive_callback[TMessage::MessageId] = [func = std::forward<F>(func), this] {
                std::lock_guard lock{s_buf_mtx};
                TMessage ret;
                std::istream is{&s_buf};
                cereal::BinaryInputArchive ar{is};
                ar(ret);
                func(std::move(ret));
                s_buf.consume(1);
            };
        } else {
            std::cout << "This id is already in use." << std::endl;
        }
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

        send_socket.async_send_to(
            boost::asio::buffer(*data), multicast_endpoint,
            [data, weak_self = std::weak_ptr{shared_from_this()}](const boost::system::error_code& error, [[maybe_unused]] size_t length) {
                if (error) {
                    std::cerr << "send failed: " << error.message() << std::endl;
                }
            });
    }

    udp::socket recv_socket;
    udp::endpoint sender_endpoint;
    udp::endpoint multicast_endpoint;
    udp::socket send_socket;
    std::unordered_map<IpPhone::Message::MessageIdType, std::function<void()>> receive_callback;

private:
    std::mutex s_buf_mtx;
    boost::asio::streambuf s_buf;
};

}  // namespace IpPhone
