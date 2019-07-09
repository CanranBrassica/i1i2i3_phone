#include <iostream>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using asio::ip::tcp;

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: receiver <port>" << std::endl;
        return 1;
    }

    asio::io_service io_service;

    io_service.post([] {
        std::cout << "do something" << std::endl;
    });

    tcp::acceptor acc(io_service, tcp::endpoint(tcp::v4(), atoi(argv[1])));
    tcp::socket socket(io_service);

    {
        boost::system::error_code error;
        acc.accept(socket, error);

        if (error) {
            std::cout << "accept failed: " << error.message() << std::endl;
        } else {
            std::cout << "accept correct!" << std::endl;
        }
    }

    // メッセージ受信
    asio::streambuf receive_buffer;
    {
        boost::system::error_code error;
        asio::read(socket, receive_buffer, asio::transfer_all(), error);
        if (error && error != asio::error::eof) {
            std::cout << "receive failed: " << error.message() << std::endl;
        } else {
            const char* data = asio::buffer_cast<const char*>(receive_buffer.data());
            std::cout << data << std::endl;
        }
    }

    io_service.run();

    return 0;
}
