#include "game.hpp"


namespace awe
{
    game_world::game_world(unsigned int seed)
        : m_rand(seed) {}

    void game_world::update()
    {
        if(completed())
            return;
        m_framecount += 1;
    }

    void render(SDL_Renderer* ren)
    {
        
    }
}
