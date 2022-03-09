#pragma once

#include <SDL.h>
#include <imgui.h>
#include <imfilebrowser.h>
#include "network.hpp"
#include "widgets.hpp"


namespace awe
{
    class application
    {
        application() = default;
    public:
        static application& instance() noexcept;

        void init(SDL_Window* win, SDL_Renderer* ren);
        void quit();

        void update_imgui();

        constexpr SDL_Window* window() const noexcept { return m_win; }
        constexpr SDL_Renderer* renderer() const noexcept { return m_ren; }

        static void report_error(
            const char* msg,
            const char* title = ""
        );

        bool started() const
        {
            return m_started;
        }
        void start(bool v = true) noexcept { m_started = v; }

        void reset()
        {
            m_network->reset();
            m_mode_panel.reset_network();
        }

        int this_player() const noexcept
        {
            switch(m_network->role())
            {
            case network::ROLE_SERVER:
                return 0;
            case network::ROLE_CLIENT:
                return 1;
            default:
                return -1;
            }
        }

        std::shared_ptr<network>& get_network() noexcept { return m_network; }

        chatroom& get_chatroom() noexcept { return m_chtrm; }
        start_panel& get_start_panel() noexcept { return m_start_panel; }

        game_world* get_game_world() const noexcept { return m_game.get(); }
        bool request_start = false;

    private:
        SDL_Window* m_win = nullptr;
        SDL_Renderer* m_ren = nullptr;

        // Network
        std::shared_ptr<network> m_network;
        bool m_started = false;

        chatroom m_chtrm;
        mode_panel m_mode_panel;
        start_panel m_start_panel;
        std::unique_ptr<game_world> m_game;
    };
}
