#include "widgets.hpp"
#include <imgui.h>
#include "main.hpp"
#include "network.hpp"


namespace awe
{
    mode_panel::mode_panel()
    {
        std::memcpy(m_ip, "127.0.0.1", 10);
    }

    void mode_panel::set_network(std::shared_ptr<network> ptr)
    {
        m_network.swap(ptr);
        m_status = m_network->get_socket().is_open() ?
            CONNECTED :
            NOT_CONNECTED;
    }

    void ShowModePanel(const char* title, mode_panel& p)
    {
        const int flags =
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_AlwaysAutoResize;
        if(ImGui::BeginPopupModal(title, nullptr, flags))
        {
            ImGui::Text("Select Your Mode");
            if(ImGui::BeginTabBar("modes"))
            {
                if(ImGui::BeginTabItem("Network Client"))
                {
                    p.client_tab();
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Network Server"))
                {
                    p.server_tab();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndPopup();
        }
    }

    void mode_panel::client_tab()
    {
        static int count = 0;
        count = ++count;
        auto& io = ImGui::GetIO();

        ImGui::InputText("IP", m_ip, 16);
        ImGui::InputInt("Port", &m_port);
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
                        m_network->connect(boost::asio::ip::address_v4::from_string(m_ip), m_port, ec);
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
            }
            break;
        case PENDING:
            using namespace std;
            ImGui::Text("%c Pending", "-\\|/"[(count / 10) % 4]);
            if(m_network_result.wait_for(10ns) == future_status::ready)
            {
                m_ec = m_network_result.get();
                m_status = m_ec ? CONNECTION_ERROR : CONNECTED;
            }
            break;
        case CONNECTION_ERROR:
            ImGui::TextColored(
                ImVec4(1, 0, 0, 1),
                "Error %d",
                m_ec.value()
            );
            if(ImGui::Button("OK"))
            {
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

        ImGui::InputInt("Port", &m_port);
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
            }
            break;
        case PENDING:
            using namespace std;
            ImGui::Text("%c Pending", "-\\|/"[(count / 10) % 4]);
            if(m_network_result.wait_for(10ns) == future_status::ready)
            {
                m_ec = m_network_result.get();
                m_status = m_ec ? CONNECTION_ERROR : CONNECTED;
            }
            break;
        case CONNECTION_ERROR:
            ImGui::TextColored(
                ImVec4(1, 0, 0, 1),
                "Error %d",
                m_ec.value()
            );
            if(ImGui::Button("OK"))
            {
                m_ec.clear();
                m_status = NOT_CONNECTED;
            }
            break;
        }
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
        if(result)
        {
            chtrm.m_ready = true;
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
                ready = v;
                sp.m_changed = true;
            }
        }
        bool enabled = sp.m_server && std::all_of(sp.m_status.begin(), sp.m_status.end(), [](const auto& v) { return v.second; });
        ImGui::BeginDisabled(!enabled);
        start = ImGui::Button("Start");
        ImGui::EndDisabled();

        ImGui::End();
        return start;
    }

    void ShowGameControl(const char* title, game_control& gc)
    {
        const int flags =
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_AlwaysAutoResize;
        if(!ImGui::Begin(title, nullptr, flags))
        {
            ImGui::End();
            return;
        }

        if(ImGui::Button("Stop"))
        {
            gc.on_stop();
        }

        ImGui::End();
    }
}
