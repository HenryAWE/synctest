#pragma once

#include <array>
#include <optional>
#include <SDL.h>


namespace awe
{
    enum class input_key
    {
        UP_1P,
        DOWN_1P,
        LEFT_1P,
        RIGHT_1P,
        UP_2P,
        DOWN_2P,
        LEFT_2P,
        RIGHT_2P,
        end
    };

    class input_manager
    {
    public:
        input_manager();

        static SDL_Keycode get_default_key(input_key k);

        SDL_Keycode get_keycode(input_key k);
        static const char* get_key_name(input_key k) noexcept;

        void reset_keys() noexcept;
        void set_key(input_key k, SDL_Keycode sdlk);
        void reset_key(input_key k);

    private:
        std::array<SDL_Keycode, static_cast<int>(input_key::end)> m_key_cfg;
    };
}
