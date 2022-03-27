#include "input.hpp"


namespace awe
{
    std::optional<input_key> input_manager::get_key(SDL_Keycode k) noexcept
    {
        switch(k)
        {
        case SDLK_w:
            return input_key::UP_1P;
        case SDLK_s:
            return input_key::DOWN_1P;
        case SDLK_a:
            return input_key::LEFT_1P;
        case SDLK_d:
            return input_key::RIGHT_1P;

        case SDLK_UP:
            return input_key::UP_2P;
        case SDLK_DOWN:
            return input_key::DOWN_2P;
        case SDLK_LEFT:
            return input_key::LEFT_2P;
        case SDLK_RIGHT:
            return input_key::RIGHT_2P;
        }

        return std::nullopt;
    }
}
