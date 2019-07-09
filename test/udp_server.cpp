#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
int main(int argc, char* argv[])
{
    try {
        boost::asio::io_service io_service;
        udp::socket socket(io_service, udp::endpoint(udp::v4(), 13));

        while (true) {
            boost::array<char, 1> recv_buf;
            udp::endpoint sender_endpoint;
            boost::system::error_code error;
            socket.receive_from(boost::asio::buffer(recv_buf),
                sender_endpoint, 0, error);

            if (error && error != boost::asio::error::message_size) {
                throw boost::system::system_error(error);
            }

            std::string message = "hoge";
            boost::system::error_code ignored_error;
            socket.send_to(boost::asio::buffer(message),
                sender_endpoint, 0, ignored_error);
            std::cout << "send: " << message << std::endl;
        }

        return 0;
    } catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}
