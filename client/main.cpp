#include <boost/asio.hpp>
#include <iostream>
#include <cereal/archives/binary.hpp>
#include <boost/algorithm/string.hpp>

#include <message.hpp>

size_t user_id;  // サーバーに接続すると割り振られる．

void start_receive(boost::asio::ip::tcp::socket& socket);

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

    std::string ip_address = argv[1];
    unsigned short port = std::atoi(argv[2]);
    socket.connect(tcp::endpoint(asio::ip::address::from_string(ip_address), port));

    std::cout << "connection success" << std::endl;

    auto tcp_send_message = [&](const auto& msg) {
        using TMessage = std::decay_t<decltype(msg)>;

        std::stringstream ss;
        {
            cereal::BinaryOutputArchive ar{ss};
            ar(TMessage::MessageId);
            ar(msg);
        }

        //        std::cout << "{";
        //        for (char c : ss.str()) {
        //            std::cout << +c << ", ";
        //        }
        //        std::cout << "}" << std::endl;

        auto data = std::make_shared<std::string>(ss.str() + IpPhone::Message::END_OF_MESSAGE);

        asio::async_write(
            socket,
            asio::buffer(*data),
            [data](const boost::system::error_code& error, size_t length) {
                if (error) {
                    std::cerr << "send failed: " << error.message() << std::endl;
                }
            });
    };

    start_receive(socket);

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
                    tcp_send_message(IpPhone::Message::ExitMessage{});
                } else if (argv[0] == "/create_room") {
                    tcp_send_message(IpPhone::Message::CreateRoom{.room_id = std::strtoul(argv[1].c_str(), nullptr, 0)});
                } else if (argv[0] == "/join_room") {
                    tcp_send_message(IpPhone::Message::JoinRoom{.room_id = std::strtoul(argv[1].c_str(), nullptr, 0)});
                } else if (argv[0] == "/leave_room") {
                    tcp_send_message(IpPhone::Message::LeaveRoom{});
                } else {
                    std::cerr << "invalid command" << std::endl;
                }
            } else {
                // ordinary message
                tcp_send_message(IpPhone::Message::TextMessage{.talker_id = user_id, .data = std::move(message)});
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

namespace
{
std::unordered_map<size_t, std::function<void()>> receive_callback;
boost::asio::streambuf recv_buf;
}  // namespace

void receive_message()
{
    std::istream is{&recv_buf};
    IpPhone::Message::MessageIdType id;
    cereal::BinaryInputArchive ar{is};
    ar(id);
    //    std::cout << "message id: " << +id << std::endl;
    assert(receive_callback[id]);
    receive_callback[id]();
    recv_buf.consume(1);  // for END_OF_MESSAGE
}

void start_receive(boost::asio::ip::tcp::socket& socket)
{
    namespace asio = boost::asio;
    asio::async_read_until(
        socket, recv_buf, IpPhone::Message::END_OF_MESSAGE,
        [&socket](const boost::system::error_code& error, size_t length) mutable {
            if (error && error != boost::asio::error::eof) {
                std::cout << "receive failed: " << error.message() << std::endl;
            }

            receive_message();
            start_receive(socket);
        });
}

template <class TMessage, class F>
int add_callback(F&& func)
{
    if (!receive_callback[TMessage::MessageId]) {
        receive_callback[TMessage::MessageId] = [func = std::forward<F>(func)] {
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

#include <boost/preprocessor/cat.hpp>

#define ADD_CALLBACK(TMessage, ...) \
    int BOOST_PP_CAT(count, __COUNTER__) = add_callback<TMessage>(__VA_ARGS__)

ADD_CALLBACK(IpPhone::Message::TextMessage, [](const IpPhone::Message::TextMessage& msg) {
    if (msg.talker_id == 0) {
        std::cout << "server: " << msg.data << std::endl;
    } else if (user_id != msg.talker_id) {
        std::cout << "user" << msg.talker_id << ": " << msg.data << std::endl;
    }
});

ADD_CALLBACK(IpPhone::Message::SetUserId, [](const IpPhone::Message::SetUserId& msg) {
    std::cout << "my user id = " << msg.user_id << std::endl;
    user_id = msg.user_id;
});
