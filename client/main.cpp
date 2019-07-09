#include <boost/asio.hpp>
#include <iostream>


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

    start_receive(socket);


    std::thread{
        [&] {
            while (true) {
                std::string message;
                boost::system::error_code error;

                std::cin >> message;

                if (message == "/exit") {
                    break;
                }

                asio::write(socket, asio::buffer(message), error);

                if (error) {
                    std::cerr << "send failed: " << error.message() << std::endl;
                }
            }
        }}
        .detach();

    io_context.run();

    socket.shutdown(tcp::socket::shutdown_send);


    return 0;
}

void receive_message(std::string msg)
{
    std::cout << "recv: " << msg << std::endl;
}

void start_receive(boost::asio::ip::tcp::socket& socket)
{
    namespace asio = boost::asio;
    static asio::streambuf recv_buf;
    asio::async_read(
        socket, recv_buf, asio::transfer_at_least(1),
        [&socket](const boost::system::error_code& error, size_t length) mutable {
            if (error && error != boost::asio::error::eof) {
                std::cout << "receive failed: " << error.message() << std::endl;
            }

            std::string msg;
            std::istream is(&recv_buf);
            is >> msg;

            receive_message(std::move(msg));

            start_receive(socket);
        });
}
