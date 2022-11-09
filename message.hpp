#pragma once

#include <string>
#include <cstddef>
#include <tuple>
#include <variant>


namespace awe
{
    enum message : std::int32_t
    {
        AWEMSG_SYNC = 0, /* int32 id; uint64 frame */
        AWEMSG_CHAT = 1, /* int32 id; string msg */
        AWEMSG_PLAYER_STATUS = 2, /* int32 id; int32 player_id; int8 status */
        AWEMSG_GAME_START = 3, /* int32 id; uint32 seed */
        AWEMSG_GAME_STOP = 4 /* int32 id */
    };

    template <message msgid>
    struct message_tuple
    {
        using type =  void;
    };
    template <>
    struct message_tuple<AWEMSG_SYNC>
    {
        using type = std::tuple<std::uint64_t>;
    };
    template <>
    struct message_tuple<AWEMSG_CHAT>
    {
        using type = std::tuple<std::string>;
    };
    template <>
    struct message_tuple<AWEMSG_PLAYER_STATUS>
    {
        using type = std::tuple<std::int32_t, std::int8_t>;
    };
    template <>
    struct message_tuple<AWEMSG_GAME_START>
    {
        using type = std::tuple<std::uint32_t>;
    };
    template <>
    struct message_tuple<AWEMSG_GAME_STOP>
    {
        using type = std::tuple<>;
    };

    typedef std::variant<
        message_tuple<AWEMSG_SYNC>::type,
        message_tuple<AWEMSG_CHAT>::type,
        message_tuple<AWEMSG_PLAYER_STATUS>::type,
        message_tuple<AWEMSG_GAME_START>::type,
        message_tuple<AWEMSG_GAME_STOP>::type
    > message_variant;
}
