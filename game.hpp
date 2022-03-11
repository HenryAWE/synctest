#pragma once

#include <cstddef>
#include <vector>
#include <random>
#include <SDL.h>


namespace awe
{
    namespace cmd
    {
        enum move_direction : std::int8_t
        {
            MV_UP = 0,
            MV_DOWN = 1,
            MV_LEFT = 2,
            MV_RIGHT = 3
        };

        struct move
        {
            std::int32_t player_id;
            move_direction dir;
        };
    }

    class game_world
    {
    public:
        game_world(unsigned int seed);

        void add_command();

        void update();

        void render(SDL_Renderer* ren);

        auto& random_engine() noexcept { return m_rand; }
        std::uint64_t framecount() { return m_framecount; }

        bool completed() const noexcept { return m_completed; }

    private:
        std::uint64_t m_framecount = 0;
        std::mt19937 m_rand;
        bool m_completed = false;
    };
}
