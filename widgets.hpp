#pragma once

#include <future>
#include <memory>
#include <string>
#include <vector>
#include <boost/system.hpp>


namespace awe
{
    class network;

    class mode_panel
    {
    public:
        mode_panel();

        void set_network(std::shared_ptr<network> ptr);

        friend void ShowModePanel(const char* title, mode_panel& p);

        enum network_status : int
        {
            NOT_CONNECTED = 0,
            CONNECTED,
            PENDING,
            CONNECTION_ERROR,
        };

        network_status get_network_status() const noexcept { return m_status; }

        bool selected() const
        {
            return get_network_status() == CONNECTED;
        }
        void reset_network()
        {
            m_status = NOT_CONNECTED;
            m_ec.clear();
        }

    private:
        std::shared_ptr<network> m_network;
        std::future<boost::system::error_code> m_network_result;
        boost::system::error_code m_ec;
        network_status m_status;
        std::string m_error_msg;

        // Cache
        char m_ip[16];
        int m_port = 10800;

        void client_tab();
        void server_tab();
    };

    class chatroom
    {
    public:
        chatroom();

        std::vector<std::string> record;

        friend bool ShowChatroom(const char* title, chatroom& chtrm);

        void clear() noexcept { std::memset(m_buf, 0, sizeof(m_buf)); }
        std::string_view get_msg() const noexcept { return m_buf; }

        bool ready() const { return m_ready && !get_msg().empty(); }
        void reset() { m_buf[0] = '\0'; m_ready = false; }

    private:
        char m_buf[512]{};
        bool m_ready = false;
    };
}
