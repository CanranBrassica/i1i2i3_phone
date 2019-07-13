#include <thread>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <deque>
#include <mutex>
#include <iostream>
#include <atomic>
#include <cmath>


struct SoxConfig
{
    std::string type = "raw";
    int bit = 16;
    int channel = 1;
    char sign_type = 's';
    int sampling_freq = 44100;
    std::string input = "-";

    std::string rec_command(bool no_output = true) const { return "rec " + config_str() + (no_output ? " 2> /dev/null " : ""); }
    std::string play_command(bool no_output = true) const { return "play " + config_str() + (no_output ? " 2> /dev/null " : ""); }

private:
    std::string config_str() const
    {
        std::stringstream ss;
        ss << " -t " << type << " -b " << bit << " -c " << channel << " -e " << sign_type << " -r " << sampling_freq << " " << input;
        return ss.str();
    }
};

struct SoundPlayer
{
    explicit SoundPlayer(const SoxConfig& sox_config)
    {
        std::cout << sox_config.play_command() << std::endl;
        play_process = popen(sox_config.play_command().c_str(), "w");
        if (!play_process) {
            throw std::runtime_error{"cannot open play process"};
        }
    }

    ~SoundPlayer()
    {
        pclose(play_process);
    }

    template <size_t N>
    void play(std::array<char, N> buf, size_t length)
    {
        fwrite(buf.data(), sizeof(char), length, play_process);
        double cov = 0;
        for (size_t i = 0; i < length; ++i) {
            cov += std::pow(buf.at(i), 2);
        }
        cov /= length;
        std::cout << cov << std::endl;
    }

    FILE* play_process;
};

struct SoundRecoder
{
    static constexpr size_t buf_size = 32;

    template <class F>
    explicit SoundRecoder(const SoxConfig& sox_config, F&& rec_callback)
    {
        std::cout << sox_config.rec_command() << std::endl;
        rec_process = popen(sox_config.rec_command().c_str(), "r");

        if (!rec_process) {
            throw std::runtime_error{"cannot open rec process"};
        }

        rec_thread = std::thread{
            [this, rec_callback = std::forward<F>(rec_callback)]() mutable {
                while (true) {
                    if (!rec_flag.load()) {
                        return;
                    }
                    std::array<char, buf_size> buf = {};
                    size_t length = fread(buf.data(), sizeof(char), buf_size, rec_process);
                    rec_callback(std::move(buf), length);
                }
            }};
    }

    ~SoundRecoder()
    {
        pclose(rec_process);
        rec_thread.join();
    }

    void stop()
    {
        rec_flag = false;
    }

private:
    FILE* rec_process;

    std::thread rec_thread;
    std::atomic<bool> rec_flag = true;
};

int main()
{
    const SoxConfig sox_config;
    SoundRecoder recoder{sox_config,
        [player = std::make_shared<SoundPlayer>(sox_config)](auto&& buf, size_t len) mutable {
            player->play(std::forward<decltype(buf)>(buf), len);
        }};

    while (true)
        ;

    return 0;
}
