#include "../client/sound.hpp"

int main()
{

    using namespace IpPhone;

    const SoxConfig sox_config;
    SoundRecoder recoder{sox_config,
        [player = std::make_shared<SoundPlayer>(sox_config)](auto&& buf, size_t len) mutable {
            player->play(std::forward<decltype(buf)>(buf), len);
        }};

    while (true)
        ;

    return 0;
}
