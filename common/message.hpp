#pragma once

#include <string>
#include <vector>
#include <functional>

#include <boost/hana.hpp>

#include <cereal/types/string.hpp>

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
    void serialize(TArchive& ar)
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

inline constexpr char END_OF_MESSAGE = -1;

}  // namespace IpPhone::Message
