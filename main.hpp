#pragma once

#include <SDL.h>
#include <imgui.h>
#include <imfilebrowser.h>
#include "network.hpp"
#include "input.hpp"
#include "widgets.hpp"
#include "runner.hpp"


namespace awe
{
    class application
    {
        application() = default;
    public:
        enum app_status
        {
            MODE_SELECT = 0,
            STARTED = 1,
            ON_ERROR = 2
        };

        static application& instance() noexcept;

        void init(SDL_Window* win, SDL_Renderer* ren);
        void quit();

        void update_imgui();
        void update_game();

        constexpr SDL_Window* window() const noexcept { return m_win; }
        constexpr SDL_Renderer* renderer() const noexcept { return m_ren; }

        static void report_error(
            const char* msg,
            const char* title = ""
        );

        bool started() const
        {
            return m_status == STARTED;
        }
        void start(std::shared_ptr<runner> r)
        {
            m_runner.swap(r);
            m_status = STARTED;
        }

        void reset()
        {
            m_network->reset();
            m_mode_panel.reset_network();
            m_runner.reset();
            m_status = MODE_SELECT;
            clear_title_info();
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

        constexpr std::shared_ptr<runner>& get_runner() noexcept
        {
            return m_runner;
        }
        constexpr input_manager& get_input_manager() noexcept { return m_input; }

        void network_error(const boost::system::error_code& ec = {});

        void set_title_info(std::string_view info);
        void clear_title_info() { set_title_info(std::string_view()); }

        constexpr std::mutex& get_mutex() noexcept { return m_mutex; }

    private:
        SDL_Window* m_win = nullptr;
        SDL_Renderer* m_ren = nullptr;
        app_status m_status;
        std::mutex m_mutex;

        void transit(app_status st);

        // Network
        std::shared_ptr<network> m_network;

        chatroom m_chtrm;
        mode_panel m_mode_panel;
        start_panel m_start_panel;
        game_control m_game_control;

        std::shared_ptr<runner> m_runner;
        input_manager m_input;
    };
}
