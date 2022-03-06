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
    }
    void application::quit()
    {
        m_win = nullptr;
        m_ren = nullptr;
    }

    void application::update_imgui()
    {
        auto& io = ImGui::GetIO();

        if(!started())
            ImGui::OpenPopup("Mode Select");
        ShowModePanel("Mode Select", m_mode_panel);
        if(m_mode_panel.selected())
        {
            m_started = true;
        }

        // Chatroom
        if(ShowChatroom("Chat", m_chtrm))
        {

        }
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
        "Synctest",
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

        // Network
        if(auto& network = app.get_network(); network && network->get_socket().is_open())
        {
            boost::system::error_code ec;
            auto& s = network->get_socket();

            // Send
            if(app.get_chatroom().ready())
            {
                auto msg = app.get_chatroom().get_msg();
                network->write<int>(AWEMSG_CHAT, ec);
                network->write(msg, ec);
                if(!ec)
                {
                    app.get_chatroom().record.emplace_back(msg);
                    app.get_chatroom().reset();
                }
            }

            // Receive
            while(network->get_socket().available())
            {
                int msgid = 0;
                network->read<int>(msgid, ec);
                switch(msgid)
                {
                case 0:
                    break;
                case AWEMSG_CHAT:
                    {
                        std::string chat_msg;
                        network->read(chat_msg, ec);
                        app.get_chatroom().record.emplace_back(std::move(chat_msg));
                    }
                    break;
                }
            }
        }

        ImGui_ImplSDL2_NewFrame(win);
        ImGui::NewFrame();

        app.update_imgui();

        ImGui::Render();

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
