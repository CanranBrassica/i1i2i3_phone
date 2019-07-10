#include <boost/asio.hpp>
#include <iostream>

std::string name;

void start_receive(boost::asio::ip::tcp::socket& socket);

int main(int argc, char* argv[])
{
    namespace asio = boost::asio;
    using tcp = asio::ip::tcp;

    if (argc != 4) {
        std::cerr << "client <server ip> <server port> <name>" << std::endl;
        return 1;
    }

    name = argv[3];

    asio::io_context io_context;

    tcp::socket socket{io_context};

    std::string ip_address = argv[1];
    unsigned short port = std::atoi(argv[2]);
    socket.connect(tcp::endpoint(asio::ip::address::from_string(ip_address), port));

    std::cout << "connection success" << std::endl;

    start_receive(socket);

    std::function<void()> read_line = [&] {
        std::string message;
        boost::system::error_code error;

        std::getline(std::cin, message);
        bool exit_flag = (message.size() > 4 && message.substr(0, 5) == "/exit");

        if (message[0] != '/') {
            message = name + ": " + message;
        }
        message += '\0';

        asio::write(socket, asio::buffer(message), error);

        if (error) {
            std::cerr << "send failed: " << error.message() << std::endl;
        } else if (exit_flag) {
            io_context.stop();
            return;
        }
        io_context.post(read_line);
    };

    io_context.post(read_line);

    // read_lineがasio::writeでブロッキングするので，とりあえずio_contextを二本立てた
    std::thread th{[&] {
        io_context.run();
    }};

    io_context.run();
    th.join();

    socket.shutdown(tcp::socket::shutdown_send);


    return 0;
}

void receive_message(std::string msg)
{
    if (msg.size() > name.size() && msg.substr(0, name.size()) == name) {
        return;
    }
    std::cout << msg << std::endl;
}

void start_receive(boost::asio::ip::tcp::socket& socket)
{
    namespace asio = boost::asio;
    static asio::streambuf recv_buf;
    asio::async_read_until(
        socket, recv_buf, '\0',
        [&socket](const boost::system::error_code& error, size_t length) mutable {
            if (error && error != boost::asio::error::eof) {
                std::cout << "receive failed: " << error.message() << std::endl;
            }

            std::string msg = asio::buffer_cast<const char*>(recv_buf.data());
            recv_buf.consume(length);

            receive_message(std::move(msg));

            start_receive(socket);
        });
}
