#include "tcp_message.hpp"
#include "udp_multicast.hpp"

#include <message.hpp>

#include <boost/asio.hpp>
#include <iostream>
#include <functional>
#include <cereal/archives/binary.hpp>
#include <boost/algorithm/string.hpp>

size_t user_id;  // サーバーに接続すると割り振られる．
std::shared_ptr<IpPhone::UdpMulticastMessage> udp_multicast_message;

void add_recv_callback(IpPhone::TcpConnection& con);
void add_recv_callback(IpPhone::UdpMulticastMessage& con);

int main(int argc, char* argv[])
{
    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;

    if (argc != 3) {
        std::cerr << "client <server ip> <server port>" << std::endl;
        return 1;
    }

    asio::io_context io_context;
    tcp::socket socket{io_context};

    IpPhone::TcpConnection tcp_connection{
        socket,
        asio::ip::address_v4::from_string(argv[1]),
        (unsigned short)std::atoi(argv[2])};

    tcp_connection.start_receive();

    std::cout << "connection success" << std::endl;

    add_recv_callback(tcp_connection);

    std::function<void()> read_line = [&] {
        while (true) {
            std::string message;

            std::getline(std::cin, message);
            bool exit_flag = (message.size() > 4 && message.substr(0, 5) == "/exit");

            if (message[0] == '/') {
                // command
                std::vector<std::string> argv;
                boost::algorithm::split(argv, message, boost::is_any_of(" ,"));
                if (argv[0] == "/exit") {
                    tcp_connection.send(IpPhone::Message::ExitMessage{});
                } else if (argv[0] == "/create_room") {
                    tcp_connection.send(IpPhone::Message::CreateRoom{.room_id = std::strtoul(argv[1].c_str(), nullptr, 0)});
                } else if (argv[0] == "/join_room") {
                    tcp_connection.send(IpPhone::Message::JoinRoom{.room_id = std::strtoul(argv[1].c_str(), nullptr, 0)});
                } else if (argv[0] == "/leave_room") {
                    tcp_connection.send(IpPhone::Message::LeaveRoom{});
                } else if (argv[0] == "/join_multicast") {
                    tcp_connection.send(IpPhone::Message::JoinMulticast{});
                } else if (argv[0] == "/leave_multicast") {
                    udp_multicast_message.reset();
                } else if (argv[0] == "/multicast_message") {
                    if (udp_multicast_message) {
                        udp_multicast_message->send(IpPhone::Message::TextMessage{.talker_id = user_id, .data = argv[1]});
                    }
                } else {
                    std::cerr << "invalid command" << std::endl;
                }
            } else {
                // ordinary message
                tcp_connection.send(IpPhone::Message::TextMessage{.talker_id = user_id, .data = std::move(message)});
            }

            if (exit_flag) {
                io_context.stop();
                return;
            }
        }
    };

    std::thread{read_line}.detach();

    io_context.run();

    socket.shutdown(tcp::socket::shutdown_send);

    return 0;
}


void add_recv_callback(IpPhone::TcpConnection& con)
{
    con.add_callback<IpPhone::Message::TextMessage>(
        [](const IpPhone::Message::TextMessage& msg) {
            if (msg.talker_id == 0) {
                std::cout << "server: " << msg.data << std::endl;
            } else if (user_id != msg.talker_id) {
                std::cout << "user" << msg.talker_id << ": " << msg.data << std::endl;
            }
        });

    con.add_callback<IpPhone::Message::SetUserId>(
        [](const IpPhone::Message::SetUserId& msg) {
            std::cout << "my user id = " << msg.user_id << std::endl;
            user_id = msg.user_id;
        });

    con.add_callback<IpPhone::Message::MulticastConfig>(
        [&](const IpPhone::Message::MulticastConfig& msg) {
            if (!udp_multicast_message) {
                std::cout << "start udp multicast. " << msg.multicast_address << ":" << msg.multicast_port << std::endl;
                udp_multicast_message = std::make_shared<IpPhone::UdpMulticastMessage>(
                    con.socket.get_io_context(),
                    boost::asio::ip::address_v4::from_string(msg.multicast_address),
                    msg.multicast_port,
                    boost::asio::ip::address_v4::from_string("0.0.0.0"));
                udp_multicast_message->start_receive();
                add_recv_callback(*udp_multicast_message);
            } else {
                std::cout << "already start udp multicast" << std::endl;
            }
        });
}

void add_recv_callback(IpPhone::UdpMulticastMessage& con)
{
    con.add_callback<IpPhone::Message::TextMessage>(
        [](const IpPhone::Message::TextMessage& msg) {
            if (msg.talker_id == 0) {
                std::cout << "[multicast] server: " << msg.data << std::endl;
            } else if (user_id != msg.talker_id) {
                std::cout << "[multicast] user" << msg.talker_id << ": " << msg.data << std::endl;
            }
        });
}
