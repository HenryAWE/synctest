#pragma once

#include <cstddef>


namespace awe
{
    enum message : std::int32_t
    {
        AWEMSG_SYNC = 0, /* int32 id; uint64 frame */
        AWEMSG_CHAT = 1, /* int32 id; string msg */
        AWEMSG_PLAYER_STATUS = 2, /* int32 id; int8 status */
        AWEMSG_GAME_START = 3, /* int32 id; uint32 seed */
        AWEMSG_GAME_STOP = 4 /* int32 id */
    };
}
