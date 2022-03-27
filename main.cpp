#include "main.hpp"
#include <boost/endian.hpp>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include "message.hpp"


namespace awe
{
    application& application::instance() noexcept
    {
        static application ins;
        return ins;
    }

    void application::init(SDL_Window* win, SDL_Renderer* ren)
    {
        m_win = win;
        m_ren = ren;
        m_network = std::make_shared<network>();
        m_mode_panel.set_network(m_network);

        m_network->register_msgproc<AWEMSG_CHAT>([](const message_tuple<AWEMSG_CHAT>::type& msg) {
            auto& chat = application::instance().get_chatroom();
            std::lock_guard guard(chat.get_mutex());
            chat.add_record(
                std::get<0>(msg),
                chat.RECV
            );
        });
        m_network->register_msgproc<AWEMSG_PLAYER_STATUS>([](const message_tuple<AWEMSG_PLAYER_STATUS>::type& msg) {
            auto& start_panel = application::instance().get_start_panel();
            std::lock_guard guard(start_panel.get_mutex());
            start_panel.set(get<0>(msg), get<1>(msg));
        });
        m_network->on_error.connect([](const boost::system::error_code& ec) {
            auto& app = application::instance();
            std::lock_guard guard(app.get_mutex());
            app.network_error(ec);
        });

        m_chtrm.on_send.connect([](std::string_view msg)->bool {
            boost::system::error_code ec;
            auto& net = *application::instance().get_network();
            {
                std::lock_guard guard(net.get_write_mutex());
                net.send_msg_chat({ msg }, ec);
            }

            if(ec)
                net.on_error(ec);
            return !ec;
        });
        m_start_panel.on_change.connect([](int id, bool state)->bool {
            boost::system::error_code ec;
            auto& net = *application::instance().get_network();
            message_tuple<AWEMSG_PLAYER_STATUS>::type msg(id, state);
            {
                std::lock_guard guard(net.get_write_mutex());
                net.send_msg<AWEMSG_PLAYER_STATUS>(msg, ec);
            }

            if(ec)
                net.on_error(ec);
            return !ec;
        });
    }
    void application::quit()
    {
        m_mode_panel.set_network(nullptr);
        m_network.reset();
        m_win = nullptr;
        m_ren = nullptr;
    }

    void application::update_imgui()
    {
        auto& io = ImGui::GetIO();

        if(!m_mode_panel.selected())
            ImGui::OpenPopup("Mode Select");
        ShowModePanel("Mode Select", m_mode_panel);
        if(m_mode_panel.get_network_status() == mode_panel::CONNECTED)
        {
            m_start_panel.set_this_id(this_player());
            if(m_network->role() == network::ROLE_SERVER)
                m_start_panel.set_server();
        }
        if(m_network->role() != network::ROLE_NONE && !started())
        {
            if(ShowStartPanel("Preparing", m_start_panel))
            {
                
            }
        }
        if(started())
        {
            ShowGameControl("Game Control", m_game_control);
        }

        // Chatroom
        ShowChatroom("Chat", m_chtrm);
    }
    void application::update_game()
    {
        if(m_status != STARTED)
            return;
    }

    void application::report_error(
        const char* msg,
        const char* title
    ) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "%s: %s",
            title, msg
        );
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            title,
            msg,
            instance().window()
        );
    }

    void application::network_error(const boost::system::error_code& ec)
    {
        get_chatroom().add_record(
            "Error " + std::to_string(ec.value()),
            chatroom::NOTIFICATION
        );
        if(m_network)
            reset();
    }

    void application::set_title_info(std::string_view info)
    {
        std::string title = "Kairos' Constancy";
        if(!info.empty())
        {
            title += " - [";
            title += info;
            title += ']';
        }
        SDL_SetWindowTitle(m_win, title.c_str());
    }

    void application::transit(app_status st)
    {
    }
}

// Will be replaced to SDL_main on Windows by the macro of SDL
int main(int argc, char* argv[])
{
    using namespace awe;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        application::report_error(SDL_GetError(), "SDL_Init() failed");
        return EXIT_FAILURE;
    }

    SDL_Window* win = SDL_CreateWindow(
        "Kairos' Constancy",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE
    );
    if(!win)
    {
        application::report_error(SDL_GetError(), "SDL_CreateWindow() failed");
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(
        win,
        -1,
        SDL_RENDERER_PRESENTVSYNC
    );
    if(!ren)
    {
        application::report_error(SDL_GetError(), "SDL_CreateRenderer() failed");
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_RendererInfo ren_info;
    SDL_GetRendererInfo(ren, &ren_info);
    SDL_RenderSetVSync(ren, 1);

    ImGuiContext* imctx = ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer_Init(ren);
    ImGui_ImplSDLRenderer_CreateDeviceObjects();

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "Renderer Name: %s",
        ren_info.name
    );

    auto& app = application::instance();
    app.init(win, ren);

    bool quit = false;
    while(!quit)
    {
        // Processing event
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;

            default:
                ImGui_ImplSDL2_ProcessEvent(&e);
                break;
            }
        }

        ImGui_ImplSDL2_NewFrame(win);
        ImGui::NewFrame();

        app.update_imgui();

        ImGui::Render();

        app.update_game();

        SDL_RenderClear(ren);
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(ren);
    }

    app.quit();

    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext(imctx);
    imctx = nullptr;

    SDL_DestroyRenderer(ren);
    ren = nullptr;
    SDL_DestroyWindow(win);
    win = nullptr;

    SDL_Quit();

    return 0;
}
