#pragma once

#include <future>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <boost/system.hpp>
#include <boost/signals2.hpp>


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

        int m_mode_id = 0;
        void client_tab();
        void server_tab();
    };

    class chatroom
    {
    public:
        chatroom();

        enum record_type : int
        {
            SEND = 1,
            RECV = 2,
            NOTIFICATION = 3
        };
        struct record_t
        {
            std::string msg;
            record_type type;

            record_t(std::string msg_, record_type type_)
                : msg(std::move(msg_)), type(type_) {}
        };

        std::vector<record_t> record;

        friend bool ShowChatroom(const char* title, chatroom& chtrm);

        void clear() noexcept { std::memset(m_buf, 0, sizeof(m_buf)); }
        std::string_view get_msg() const noexcept { return m_buf; }

        bool ready() const { return m_ready && !get_msg().empty(); }
        void reset() { m_buf[0] = '\0'; m_ready = false; }

        void add_record(std::string msg, record_type type)
        {
            record.emplace_back(std::move(msg), type);
        }

    private:
        char m_buf[512]{};
        bool m_ready = false;
    };

    class start_panel
    {
    public:
        typedef std::pair<int, bool> player_status_t;

        void set(int player_id, bool ready)
        {
            if(player_id > 2)
                throw std::out_of_range("player ID out of range");
            m_status[player_id] = ready;
        }
        void set_this_id(int player_id)
        {
            m_this_id = player_id;
        }
        bool get(int player_id) const
        {
            return m_status.at(player_id);
        }
        bool changed() const noexcept { return m_changed; }
        void changed(bool v) noexcept { m_changed = v; }

        void set_server() { m_server = true; }

        friend bool ShowStartPanel(const char* title, start_panel& sp);

    private:
        std::map<int, bool> m_status;
        int m_this_id;
        bool m_changed = false;
        bool m_server = false;
    };

    class game_world;

    class game_control
    {
    public:
        ~game_control();

        friend void ShowGameControl(const char* title, game_control& gc);

        void set_game_world(std::shared_ptr<game_world> ptr)
        {
            m_game_world.swap(ptr);
        }

        boost::signals2::signal<void()> on_stop;

    private:
        std::shared_ptr<game_world> m_game_world;
    };
}
