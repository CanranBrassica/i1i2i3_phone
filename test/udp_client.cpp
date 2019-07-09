#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
int main(int argc, char* argv[])
{
//    try {
        if (argc != 3) {
            std::cerr << "Usage: client <host> <port>" << std::endl;
            return 1;
        }

        boost::asio::io_service io_service;
        udp::endpoint receiver_endpoint(boost::asio::ip::address::from_string(argv[1]), atoi(argv[2]));
        std::cout << receiver_endpoint << std::endl;

        udp::socket socket(io_service);
        socket.open(udp::v4());

        boost::array<char, 1> send_buf = {0};
        socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);

        std::array<char, 128> recv_buf;
        udp::endpoint sender_endpoint;
        size_t len = socket.receive_from(
            boost::asio::buffer(recv_buf), sender_endpoint);

        std::cout.write(recv_buf.data(), len);

        return 0;
//    } catch (std::exception& ex) {
//        std::cerr << ex.what() << std::endl;
//        return 1;
//    }
}
