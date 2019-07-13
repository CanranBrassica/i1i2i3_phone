#pragma once

#include <string>
#include <vector>
#include <cereal/types/string.hpp>
#include <cereal/types/array.hpp>
#include <boost/asio.hpp>

namespace IpPhone::Message
{

using MessageIdType = char;

struct TextMessage
{
    static inline constexpr MessageIdType MessageId = 0;

    size_t talker_id;
    std::string data;

    template <class TArchive>
    void serialize(TArchive& ar)
    {
        ar(talker_id, data);
    }
};

struct ExitMessage
{
    static inline constexpr MessageIdType MessageId = 1;

    template <class TArchive>
    void serialize(TArchive&)
    {}
};

struct CreateRoom
{
    static inline constexpr MessageIdType MessageId = 2;

    size_t room_id;

    template <class TArchive>
    void serialize(TArchive& ar)
    {
        ar(room_id);
    }
};

struct JoinRoom
{
    static inline constexpr MessageIdType MessageId = 3;

    size_t room_id;

    template <class TArchive>
    void serialize(TArchive& ar)
    {
        ar(room_id);
    }
};

struct SetUserId
{
    static inline constexpr MessageIdType MessageId = 4;

    size_t user_id;

    template <class TArchive>
    void serialize(TArchive& ar)
    {
        ar(user_id);
    }
};

struct LeaveRoom
{
    static inline constexpr MessageIdType MessageId = 5;

    template <class TArchive>
    void serialize(TArchive&)
    {}
};

// UDP Multicast通信に参加したいときにclientが送る
// Multicast Configが返ってくるので，それを用いてUDP Multicast通信に参加する
struct JoinMulticast
{
    static inline constexpr MessageIdType MessageId = 6;

    template <class TArchive>
    void serialize(TArchive&)
    {}
};

struct MulticastConfig
{
    static inline constexpr MessageIdType MessageId = 7;

    std::string multicast_address;
    unsigned short multicast_port;

    template <class TArchive>
    void serialize(TArchive& ar)
    {
        ar(multicast_address, multicast_port);
    }
};

struct CallStart
{
    static inline constexpr MessageIdType MessageId = 8;

    template <class TArchive>
    void serialize(TArchive&)
    {}
};

struct PhoneData
{
    static inline constexpr MessageIdType MessageId = 9;
    static inline constexpr size_t DataSize = 256;

    size_t talker_id;
    std::array<char, DataSize> data;
    size_t length;

    template <class TArchive>
    void serialize(TArchive& ar)
    {
        ar(talker_id, data, length);
    }
};


inline constexpr char END_OF_MESSAGE = -1;

}  // namespace IpPhone::Message
