#pragma once

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
        RIGHT_2P
    };

    class input_manager
    {
    public:
        static std::optional<input_key> get_key(SDL_Keycode k) noexcept;
    };
}
