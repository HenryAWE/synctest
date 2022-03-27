#pragma once

#include <cstddef>
#include <vector>
#include <variant>
#include <array>
#include <queue>
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
        typedef std::variant<
            cmd::move
        > cmd_t;

        game_world(unsigned int seed);

        template <typename Cmd>
        void add_command(Cmd command)
        {
            m_cmds.push(command);
        }

        void update();

        void render(SDL_Renderer* ren);

        auto& random_engine() noexcept { return m_rand; }
        std::uint64_t framecount() { return m_framecount; }

        bool completed() const noexcept { return m_completed; }

    private:
        std::uint64_t m_framecount = 0;
        std::mt19937 m_rand;
        bool m_completed = false;
        std::queue<cmd_t> m_cmds; // commands
    };
}
