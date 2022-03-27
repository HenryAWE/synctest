#include "runner.hpp"


namespace awe
{
    local_multi_runner::local_multi_runner()
    {
        std::random_device dev;
        m_game = std::make_shared<game_world>(dev());
    }
}
