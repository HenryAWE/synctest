#pragma once

#include <mutex>
#include <future>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <map>
#include <boost/system.hpp>
#include <boost/signals2.hpp>


namespace awe
{
    class network;

    class mode_panel
    {
    public:
        enum mode
        {
            MODE_NONE = 0,
            MODE_LOCAL_SINGLE_PLAYER = 1,
            MODE_LOCAL_DOUBLE_PLAYER = 2,
            MODE_REPLAY,
            MODE_NETWORK,
        };

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

        mode get_mode() const
        {
            return m_mode;
        }

    private:
        std::shared_ptr<network> m_network;
        std::future<boost::system::error_code> m_network_result;
        boost::system::error_code m_ec;
        network_status m_status;
        std::string m_error_msg;
        std::optional<std::string> m_app_info;

        // Cache
        char m_ip[16];
        int m_port = 10800;

        int m_mode_id = 0;
        mode m_mode = MODE_NONE;
        void local_single_tab();
        void local_double_tab();
        void replay_tab();
        void client_tab();
        void server_tab();
        void settings_tab();
        void about_tab();

        bool freeze_ui() const noexcept;

        void build_app_info();
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

        void reset() { m_buf[0] = '\0'; }

        void add_record(std::string msg, record_type type)
        {
            record.emplace_back(std::move(msg), type);
        }

        constexpr std::mutex& get_mutex() noexcept { return m_mutex; }

        boost::signals2::signal<bool(std::string_view)> on_send;

    private:
        std::mutex m_mutex;
        char m_buf[512]{};
    };

    class start_panel
    {
    public:
        typedef std::pair<int, bool> player_status_t;

        start_panel();

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

        void set_server() { m_server = true; }

        friend bool ShowStartPanel(const char* title, start_panel& sp);

        constexpr std::mutex& get_mutex() noexcept { return m_mutex; }

        boost::signals2::signal<bool(int, bool)> on_change;

    private:
        std::mutex m_mutex;
        std::map<int, bool> m_status;
        int m_this_id;
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
