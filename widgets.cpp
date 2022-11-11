#include "widgets.hpp"
#include <boost/locale.hpp>
#include <imgui.h>
#include "main.hpp"
#include "network.hpp"
#include "game.hpp"


namespace awe
{
    namespace detail
    {
        std::string get_ec_message(const boost::system::error_code& ec)
        {
#ifdef _WIN32
            boost::locale::generator gen;
            auto loc = gen(boost::locale::util::get_system_locale());
            return boost::locale::conv::to_utf<char>(
                ec.message(),
                loc
            );
#else
            return ec.message();
#endif
        }
    }

    mode_panel::mode_panel()
    {
        std::memcpy(m_ip, "127.0.0.1", 10);
    }

    void mode_panel::set_network(std::shared_ptr<network> ptr)
    {
        m_network.swap(ptr);
        if(!m_network)
            return;
        m_status = m_network->get_socket().is_open() ?
            CONNECTED :
            NOT_CONNECTED;
    }

    void ShowModePanel(const char* title, mode_panel& p)
    {
        auto& io = ImGui::GetIO();

        const int flags =
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_AlwaysAutoResize;

        const char* const modes_name[] =
        {
            "Local Single Player",
            "Local Double Player",
            "Replay",
            "Network Client",
            "Network Server",
            "Settings",
            "About",
            "Quit"
        };
        decltype(&mode_panel::server_tab) funcs[] =
        {
            &mode_panel::local_single_tab,
            &mode_panel::local_double_tab,
            &mode_panel::replay_tab,
            &mode_panel::client_tab,
            &mode_panel::server_tab,
            &mode_panel::settings_tab,
            &mode_panel::about_tab
        };

        ImGui::SetNextWindowPos(
            ImVec2(io.DisplaySize.x / 2.0f, io.DisplaySize.y * 0.2f),
            ImGuiCond_Always,
            ImVec2(0.5f, 0.0f)
        );
        if(ImGui::BeginPopupModal(title, nullptr, flags))
        {
            ImGui::BeginDisabled(p.freeze_ui());
            ImGui::Combo("Select Your Mode", &p.m_mode_id, modes_name, std::size(modes_name));
            ImGui::EndDisabled();
            ImGui::Separator();
            if(p.m_mode_id == std::size(funcs))
            {
                application::instance().quit();
            }
            else
                (p.*funcs[p.m_mode_id])();
            ImGui::EndPopup();
        }
    }

    void mode_panel::local_single_tab()
    {

    }
    void mode_panel::local_double_tab()
    {
        ImGui::Text("1P: WASD\n2P: Arrow Keys\n (See Settings)");
        if(ImGui::Button("Start"))
        {
            application::instance().start(
                std::make_shared<local_multi_runner>()
            );
            ImGui::CloseCurrentPopup();
        }
    }
    void mode_panel::replay_tab()
    {

    }
    void mode_panel::client_tab()
    {
        static int count = 0;
        count = ++count;
        auto& io = ImGui::GetIO();

        ImGui::BeginDisabled(freeze_ui());
        ImGui::InputText("IP", m_ip, 16);
        ImGui::InputInt("Port", &m_port);
        ImGui::EndDisabled();
        switch(m_status)
        {
        case NOT_CONNECTED:
            if(ImGui::Button("Connect"))
            {
                assert(!m_network_result.valid());
                m_network_result = std::async(
                    std::launch::async,
                    [this]()
                    {
                        boost::system::error_code ec;
                        auto addr = boost::asio::ip::address_v4::from_string(m_ip, ec);
                        if(ec)
                            return ec;
                        m_network->connect(addr, m_port, ec);
                        return ec;
                    }
                );
                m_status = PENDING;
            }
            break;
        case CONNECTED:
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Connected");
            if(ImGui::Button("OK"))
            {
                ImGui::CloseCurrentPopup();
                auto remote_ep = m_network->get_socket().remote_endpoint();
                application::instance().get_chatroom().add_record(
                    "Connected to server " +
                    remote_ep.address().to_string() +
                    " on port " +
                    std::to_string(remote_ep.port()),
                    chatroom::NOTIFICATION
                );
                application::instance().set_title_info("client");
            }
            break;
        case PENDING:
            using namespace std;
            ImGui::Text("%c Pending", "-\\|/"[(count / 10) % 4]);
            if(m_network_result.wait_for(10ns) == future_status::ready)
            {
                m_ec = m_network_result.get();
                if(m_ec)
                {
                    if(m_ec.value() == boost::asio::error::interrupted || m_ec.value() == boost::asio::error::operation_aborted)
                        m_status = NOT_CONNECTED;
                    else
                    {
                        m_status = CONNECTION_ERROR;
                        m_error_msg = detail::get_ec_message(m_ec);
                    }
                }
                else
                    m_status = CONNECTED;
            }
            else
            {
                ImGui::SameLine();
                if(ImGui::Button("Cancel"))
                {
                    m_network->cancel_connect();
                }
            }
            break;
        case CONNECTION_ERROR:
            ImGui::TextColored(
                ImVec4(1, 0, 0, 1),
                "Error: %s (%d, 0x%X)",
                m_error_msg.c_str(),
                m_ec.value(),
                m_ec.value()
            );
            if(ImGui::Button("OK"))
            {
                m_error_msg.clear();
                m_ec.clear();
                m_status = NOT_CONNECTED;
            }
            break;
        }
    }
    void mode_panel::server_tab()
    {
        static int count = 0;
        count = ++count;
        auto& io = ImGui::GetIO();

        ImGui::BeginDisabled(freeze_ui());
        ImGui::InputInt("Port", &m_port);
        ImGui::EndDisabled();
        switch(m_status)
        {
        case NOT_CONNECTED:
            if(ImGui::Button("Accept"))
            {
                assert(!m_network_result.valid());
                m_network_result = std::async(
                    std::launch::async,
                    [this]()
                    {
                        boost::system::error_code ec;
                        m_network->accept(m_port, ec);
                        return ec;
                    }
                );
                m_status = PENDING;
            }
            break;
        case CONNECTED:
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Connected");
            if(ImGui::Button("OK"))
            {
                ImGui::CloseCurrentPopup();
                auto remote_ep = m_network->get_socket().remote_endpoint();
                application::instance().get_chatroom().add_record(
                    "Accepted connection from " +
                    remote_ep.address().to_string() +
                    " on port " +
                    std::to_string(remote_ep.port()),
                    chatroom::NOTIFICATION
                );
                application::instance().set_title_info("server");
            }
            break;
        case PENDING:
            using namespace std;
            ImGui::Text("%c Pending", "-\\|/"[(count / 10) % 4]);
            if(m_network_result.wait_for(10ns) == future_status::ready)
            {
                m_ec = m_network_result.get();
                if(m_ec)
                {
                    if(m_ec.value() == boost::asio::error::interrupted || m_ec.value() == boost::asio::error::operation_aborted)
                        m_status = NOT_CONNECTED;
                    else
                    {
                        m_status = CONNECTION_ERROR;
                        m_error_msg = detail::get_ec_message(m_ec);
                    }
                }
                else
                    m_status = CONNECTED;
            }
            else
            {
                ImGui::SameLine();
                if(ImGui::Button("Cancel"))
                {
                    m_network->cancel_accept();
                }
            }
            break;
        case CONNECTION_ERROR:
            ImGui::TextColored(
                ImVec4(1, 0, 0, 1),
                "Error: %s (%d, 0x%X)",
                m_error_msg.c_str(),
                m_ec.value(),
                m_ec.value()
            );
            if(ImGui::Button("OK"))
            {
                m_error_msg.clear();
                m_ec.clear();
                m_status = NOT_CONNECTED;
            }
            break;
        }
    }
    void mode_panel::settings_tab()
    {
        if(!ImGui::BeginChild("#settings", ImVec2(0, ImGui::GetIO().DisplaySize.y * 0.4f)))
        {
            ImGui::EndChild();
            return;
        }
        if(ImGui::TreeNodeEx("Input", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto& im = application::instance().get_input_manager();
            if(ImGui::TreeNodeEx("Key", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for(int i = 0; i < (int)input_key::end; ++i)
                {
                    auto k = static_cast<input_key>(i);
                    ImGui::Text(
                        "%s - %s",
                        im.get_key_name(k),
                        SDL_GetKeyName(im.get_keycode(k))
                    );
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        ImGui::EndChild();
    }
    void mode_panel::about_tab()
    {
        if(!m_app_info.has_value())
            build_app_info();
        const auto& str = *m_app_info;

        if(ImGui::BeginChild("#settings", ImVec2(0, ImGui::GetIO().DisplaySize.y * 0.4f)))
        {
            ImGui::TextUnformatted(
                std::to_address(str.begin()),
                std::to_address(str.end())
            );
        }
        ImGui::EndChild();
    }

    bool mode_panel::freeze_ui() const noexcept
    {
        return m_status != NOT_CONNECTED;
    }

    void mode_panel::build_app_info()
    {
        std::stringstream ss;
        ss << std::format(
            "Compiler: {}\n"
            "Standard Library: {}\n",
            BOOST_COMPILER,
            BOOST_STDLIB
        );
        ss << "__cplusplus: " << __cplusplus << "L\n";
#ifdef _DEBUG
        ss << "_DEBUG: " << _DEBUG << std::endl;
#endif

        SDL_version sdl_ver{};
        SDL_VERSION(&sdl_ver);
        ss << std::format(
            "SDL Version: {}.{}.{}\n",
            sdl_ver.major,
            sdl_ver.minor,
            sdl_ver.patch
        );
        SDL_GetVersion(&sdl_ver);
        ss << std::format(
            "SDL Version (Runtime): {}.{}.{}\n",
            sdl_ver.major,
            sdl_ver.minor,
            sdl_ver.patch
        );

        ss << std::format(
            "Boost Version {}.{}.{} ({})\n",
            BOOST_VERSION / 100000,
            BOOST_VERSION % 10000 / 100,
            BOOST_VERSION % 100,
            BOOST_VERSION
        );

        ss << std::format(
            "ImGui Version: {} ({})\n",
            IMGUI_VERSION, IMGUI_VERSION_NUM
        );

        auto* ren = application::instance().renderer();
        SDL_RendererInfo ren_info{};
        SDL_GetRendererInfo(ren, &ren_info);
        ss << std::format(
            "Renderer\n"
            "  name: {}\n"
            "  max texture size: {} x {}",
            ren_info.name,
            ren_info.max_texture_width, ren_info.max_texture_height
        );

        m_app_info = std::move(ss).str();
    }

    chatroom::chatroom()
    {
        reset();
    }

    bool ShowChatroom(const char* title, chatroom& chtrm)
    {
        ImGui::SetNextWindowSize(ImVec2(400.0f, 400.0f), ImGuiCond_FirstUseEver);
        if(!ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::End();
            return false;
        }

        if(ImGui::BeginChild("texts", ImVec2(0, -24.0f), true))
        {
            std::lock_guard guard(chtrm.get_mutex());
            for(auto& i : chtrm.record)
            {
                switch(i.type)
                {
                case chatroom::SEND:
                    ImGui::TextColored(ImVec4(0, 1, 1, 1), "SEND >>:");
                    break;
                case chatroom::RECV:
                    ImGui::TextColored(ImVec4(0, 0, 1, 1), "RECV <<:");
                    break;

                default:
                case chatroom::NOTIFICATION:
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "[INFO] ");
                    break;
                }
                ImGui::SameLine();
                ImGui::Text("%s", i.msg.c_str());
            }
        }
        ImGui::EndChild();
        ImGui::InputText("Input", chtrm.m_buf, sizeof(chtrm.m_buf));
        ImGui::SameLine();
        bool result = ImGui::Button("Send");
        if(result && !chtrm.get_msg().empty())
        {
            if(chtrm.on_send(chtrm.get_msg()))
            {
                chtrm.add_record(
                    std::string(chtrm.get_msg()),
                    chtrm.SEND
                );
                chtrm.clear();
            }
        }

        ImGui::End();

        return result;
    }

    bool ShowStartPanel(const char* title, start_panel& sp)
    {
        const int flags =
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        if(!ImGui::Begin(title, nullptr, flags))
        {
            ImGui::End();
            return false;
        }

        ImGui::Text("You are %dP now", sp.m_this_id + 1);

        bool start = false;
        std::lock_guard guard(sp.get_mutex());
        for(auto& [id, ready] : sp.m_status)
        {
            const char label[] = { char('1' + id), 'P', '\0'};
            bool v = ready;
            ImGui::BeginDisabled(id != sp.m_this_id);
            ImGui::Checkbox(label, &v);
            ImGui::SameLine();
            ImGui::EndDisabled();
            if(id == sp.m_this_id)
            {
                if(sp.on_change(id, v))
                    ready = v;
            }
        }
        bool enabled = sp.m_server && std::all_of(sp.m_status.begin(), sp.m_status.end(), [](const auto& v) { return v.second; });
        ImGui::BeginDisabled(!enabled);
        start = ImGui::Button("Start");
        ImGui::EndDisabled();

        ImGui::End();
        return start;
    }

    game_control::~game_control() = default;

    void ShowGameControl(const char* title, game_control& gc)
    {
        const int flags =
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        if(!ImGui::Begin(title, nullptr, flags))
        {
            ImGui::End();
            return;
        }

        auto& io = ImGui::GetIO();
        ImGui::Text("Framecount: %u", (unsigned int)gc.m_game_world->framecount());
        ImGui::Text("FPS: %.1f", io.Framerate);

        if(ImGui::Button("Stop"))
        {
            gc.on_stop();
        }

        ImGui::End();
    }

    start_panel::start_panel()
    {
        set(0, false);
        set(1, false);
    }
}
