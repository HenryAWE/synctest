#include "input.hpp"
#include <stdexcept>


namespace awe
{
    input_manager::input_manager()
    {
        reset_keys();
    }

    SDL_Keycode input_manager::get_default_key(input_key k)
    {
        switch(k)
        {
            using enum input_key;
        case UP_1P:
            return SDLK_w;
        case DOWN_1P:
            return SDLK_s;
        case LEFT_1P:
            return SDLK_a;
        case RIGHT_1P:
            return SDLK_d;
        case UP_2P:
            return SDLK_UP;
        case DOWN_2P:
            return SDLK_DOWN;
        case LEFT_2P:
            return SDLK_LEFT;
        case RIGHT_2P:
            return SDLK_RIGHT;
        default:
            throw std::invalid_argument("invalid key enum");
        }
    }

    SDL_Keycode input_manager::get_keycode(input_key k)
    {
        return m_key_cfg[static_cast<std::size_t>(k)];
    }
    const char* input_manager::get_key_name(input_key k) noexcept
    {
        switch(k)
        {
            using enum input_key;
        case UP_1P:
            return "Up (Player 1)";
        case DOWN_1P:
            return "Down (Player 1)";
        case LEFT_1P:
            return "Left (Player 1)";
        case RIGHT_1P:
            return "Right (Player 1)";
        case UP_2P:
            return "Up (Player 2)";
        case DOWN_2P:
            return "Down (Player 2)";
        case LEFT_2P:
            return "Left (Player 2)";
        case RIGHT_2P:
            return "Right (Player 2)";
        default:
            return "Unknown";
        }
    }

    void input_manager::reset_keys() noexcept
    {
        for(std::size_t i = 0; i < std::size(m_key_cfg); ++i)
        {
            m_key_cfg[i] = get_default_key(static_cast<input_key>(i));
        }
    }
    void input_manager::set_key(input_key k, SDL_Keycode sdlk)
    {
        m_key_cfg.at(static_cast<std::size_t>(k)) = sdlk;
    }
    void input_manager::reset_key(input_key k)
    {
        m_key_cfg.at(static_cast<std::size_t>(k)) = get_default_key(k);
    }
}
