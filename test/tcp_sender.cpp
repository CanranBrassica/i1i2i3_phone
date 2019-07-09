
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
namespace asio = boost::asio;

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: sender <host> <port>" << std::endl;
        return 1;
    }

    boost::asio::io_service io_service;
    tcp::endpoint receiver_endpoint(boost::asio::ip::address::from_string(argv[1]), atoi(argv[2]));
    std::cout << receiver_endpoint << std::endl;

    io_service.post([] {
        std::cout << "do something" << std::endl;
    });

    tcp::socket socket(io_service);
    socket.connect(receiver_endpoint);

    std::string msg = "This is message!!";
    boost::system::error_code error;
    asio::write(socket, asio::buffer(msg), error);
    if (error) {
        std::cout << "send failed: " << error.message() << std::endl;
    } else {
        std::cout << "send correct!" << std::endl;
    }

    io_service.run();

    return 0;
}
