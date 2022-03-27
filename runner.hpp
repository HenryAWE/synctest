#pragma once

#include <memory>
#include "game.hpp"
#include "input.hpp"


namespace awe
{
    class runner
    {
    public:
        virtual ~runner() = default;
    };

    //  Local multi players
    class local_multi_runner : public runner
    {
    public:
        local_multi_runner();

        std::shared_ptr<game_world>& game() noexcept { return m_game; }

    private:
        std::shared_ptr<game_world> m_game;
    };
}
