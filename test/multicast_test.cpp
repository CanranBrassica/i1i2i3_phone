#include "../common/message.hpp"

#include "../client/udp_multicast.hpp"


#include <iostream>

int main()
{

    using namespace boost::asio;

    ip::address_v4 multicast_address = ip::address_v4::from_string("229.0.0.1");
    unsigned short port = 5000;
    ip::address_v4 network_interface = ip::address_v4::from_string("0.0.0.0");

    boost::asio::io_context ioc;
    IpPhone::UdpMulticastMessage udp_mul{
        ioc,
        multicast_address,
        port,
        network_interface};

    udp_mul.add_callback<IpPhone::Message::TextMessage>([](const IpPhone::Message::TextMessage& msg) {
        std::cout << msg.data << std::endl;
    });

    std::thread{
        [&] {
            while (true) {
                std::string message;
                std::getline(std::cin, message);
                udp_mul.send(IpPhone::Message::TextMessage{.data = message});
            }
        }}
        .detach();

    ioc.run();
}
