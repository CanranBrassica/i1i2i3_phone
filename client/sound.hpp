#pragma once

#include <string>
#include <sstream>
#include <atomic>
#include <thread>

namespace IpPhone
{

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
    }

    FILE* play_process;
};

struct SoundRecoder
{
    static constexpr size_t buf_size = 1024;

    template <class F>
    explicit SoundRecoder(const SoxConfig& sox_config, F&& rec_callback)
    {
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
        stop();
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
}  // namespace IpPhone
