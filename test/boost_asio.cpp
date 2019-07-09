#include <boost/asio.hpp>
#include <iostream>

int main()
{

    boost::asio::io_service ios;

    auto start = std::chrono::system_clock::now();
    for (std::size_t i = 0; i < 2; ++i) {
        ios.post(
            [&] {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                std::cout << (std::chrono::system_clock::now() - start).count() / 1e9 << std::endl;
                std::cout << "hoge" << std::endl;
            });
        ios.post(
            [&] {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << (std::chrono::system_clock::now() - start).count() / 1e9 << std::endl;
                std::cout << "fuga" << std::endl;
            });
    }

    std::thread th{
        [&ios] {
            ios.run();
        }};

    ios.run();

    th.join();

    return 0;
}