#pragma once

#include <cstddef>
#include <atomic>
#include <type_traits>
#include <thread>
#include <future>
#include <map>
#ifdef _WIN32
#   include <sdkddkver.h>
#endif
#include <boost/asio.hpp>
#include <boost/endian.hpp>
#include <boost/signals2.hpp>
#include "message.hpp"


namespace awe
{
    class network
    {
    public:
        network();

        virtual ~network();

        boost::asio::ip::tcp::socket& get_socket() { return m_sock; }

        constexpr std::mutex& get_write_mutex() noexcept { return m_write_mutex; }

        template <typename T>
        void write(const T& val, boost::system::error_code& ec)
        {
            using U = std::remove_cvref_t<T>;
            if constexpr(std::is_integral_v<U>)
            {
                union data_t
                {
                    U value;
                    std::byte buffer[sizeof(U)];
                };
                data_t data;
                data.value = boost::endian::native_to_little(val);
                write_buf(data.buffer, sizeof(data.buffer), ec);
            }
            else if constexpr(std::is_floating_point_v<U>)
            {
                union data_t
                {
                    U value;
                    std::byte buffer[sizeof(U)];
                };
                data_t data;
                write_buf(data.buffer, sizeof(data.buffer), ec);
            }
            else
            {
                static_assert(false, "Error");
            }
        }
        void write(std::string_view view, boost::system::error_code& ec)
        {
            write<std::uint64_t>(view.length(), ec);
            write_buf(view.data(), view.length(), ec);
        }

        void write_buf(const void* data, std::size_t len, boost::system::error_code& ec);

        template <typename T>
        void read(T& out, boost::system::error_code& ec)
        {
            using U = std::remove_cvref_t<T>;
            if constexpr(std::is_integral_v<U>)
            {
                union data_t
                {
                    U value;
                    std::byte buffer[sizeof(U)];
                };
                data_t data;
                m_sock.read_some(boost::asio::buffer(data.buffer, sizeof(data.buffer)), ec);
                out = boost::endian::little_to_native(data.value);
            }
            else if constexpr(std::is_same_v<U, std::string>)
            {
                std::uint64_t len;
                read(len, ec);
                if(ec)
                    return;
                out.clear();
                out.resize(len);
                m_sock.read_some(boost::asio::buffer(out), ec);
            }
            else
            {
                static_assert(false, "Error");
            }
        }

        template <message msgid>
        void send_msg(const typename message_tuple<msgid>::type& msg, boost::system::error_code& ec)
        {
            write<std::underlying_type_t<message>>(msgid, ec);
            if(ec)
                return;
            std::apply(
                [this, &ec](auto&&... args) { (write(args, ec), ...); },
                msg
            );
        }
        void send_msg_chat(const std::tuple<std::string_view>& msg, boost::system::error_code& ec)
        {
            write<std::underlying_type_t<message>>(AWEMSG_CHAT, ec);
            if(ec)
                return;
            write(get<0>(msg), ec);
        }

        template <message msgid>
        message_tuple<msgid>::type recv_msg(boost::system::error_code& ec)
        {
            using T = message_tuple<msgid>::type;
            T ret;
            std::apply(
                [this, &ec](auto&... args) {(read(args, ec), ...); },
                ret
            );
            return std::move(ret);
        }

        void connect(
            const boost::asio::ip::address& addr,
            unsigned short port,
            boost::system::error_code& ec
        );
        void accept(
            unsigned short port,
            boost::system::error_code& ec
        );
        void cancel_connect() noexcept;
        void cancel_accept();

        enum network_role
        {
            ROLE_NONE = 0,
            ROLE_SERVER = 1,
            ROLE_CLIENT = 2
        };

        network_role role() const noexcept { return m_role; }
        void reset();

        template <message MsgId, typename Func>
        auto register_msgproc(Func&& func)
        {
            return m_callbacks[MsgId].connect([f = std::forward<Func>(func)](const message_variant& msg) mutable {
                f(std::get<static_cast<std::size_t>(MsgId)>(msg));
            });
        }
        boost::signals2::signal<void(const boost::system::error_code&)> on_error;

    protected:
        boost::asio::io_service m_service;
        boost::asio::ip::tcp::acceptor m_acc;
        boost::asio::ip::tcp::socket m_sock;
        network_role m_role = ROLE_NONE;
        std::mutex m_write_mutex;

        std::map<message, boost::signals2::signal<void(const message_variant&)>> m_callbacks;

        std::atomic_bool m_launched;
        std::jthread m_msg_thread;
        std::thread::id m_msg_thread_id;

        void launch_msgproc_thread();

        void msgproc_main(std::stop_token stop);
        void proc_msg(message msgid, boost::system::error_code& ec);
    };
}
