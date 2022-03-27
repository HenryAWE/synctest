#include "network.hpp"
#include <boost/asio/impl/src.hpp>


namespace awe
{
    network::network()
        : m_service(1), m_acc(m_service), m_sock(m_service) {}

    network::~network()
    {
        assert(!m_msg_thread.joinable());
        m_msg_thread.request_stop();
    }

    void network::write_buf(const void* data, std::size_t len, boost::system::error_code& ec)
    {
        m_sock.write_some(boost::asio::buffer(data, len), ec);
    }

    void network::connect(
            const boost::asio::ip::address& addr,
            unsigned short port,
            boost::system::error_code& ec
    ) {
        namespace asio = boost::asio;

        boost::asio::ip::tcp::endpoint ep(addr, port);
        m_sock.connect(ep, ec);
        if(!ec)
        {
            m_role = ROLE_CLIENT;
            std::call_once(m_launched, &network::launch_msgproc_thread, this);
        }
        else
            m_sock.close();
    }
    void network::accept(
        unsigned short port,
        boost::system::error_code& ec
    ) {
        namespace asio = boost::asio;
        asio::ip::tcp::endpoint ep(asio::ip::address(), port);
        m_acc = asio::ip::tcp::acceptor(m_service, ep);
        m_acc.accept(m_sock, ec);
        if(!ec)
        {
            m_role = ROLE_SERVER;
            std::call_once(m_launched, &network::launch_msgproc_thread, this);
        }
        else
            m_sock.close();
    }
    void network::cancel_connect()
    {
        m_sock.cancel();
        m_sock.close();
    }
    void network::cancel_accept()
    {
        m_acc.cancel();
        m_acc.close();
        m_sock.close();
    }

    void network::reset()
    {
        m_role = ROLE_NONE;
        m_sock.close();
        m_acc.close();
        m_service.reset();
    }

    void network::launch_msgproc_thread()
    {
        m_msg_thread = std::jthread(
            std::bind(&network::msgproc_main, this, std::placeholders::_1)
        );
        m_msg_thread.detach();
    }

    void network::msgproc_main(std::stop_token stop)
    {
        boost::system::error_code ec;
        while(!stop.stop_requested())
        {
            if(m_sock.is_open() && m_sock.available(ec))
            {
                if(ec) // Error code of m_sock.available(ec)
                {
                    on_error(ec);
                    ec.clear();
                    continue;
                }
                std::int32_t msgid = 0;
                read<std::int32_t>(msgid, ec);
                if(ec)
                {
                    on_error(ec);
                    ec.clear();
                    continue;
                }
                proc_msg(static_cast<message>(msgid), ec);
            }
        }
    }
    void network::proc_msg(message msgid, boost::system::error_code& ec)
    {
        switch(msgid)
        {
            case AWEMSG_CHAT:
            {
                auto msg = recv_msg<AWEMSG_CHAT>(ec);
                if(!ec)
                {
                    m_callbacks[AWEMSG_CHAT](std::move(msg));
                }
            }
            break;
            case AWEMSG_PLAYER_STATUS:
            {
                auto msg = recv_msg<AWEMSG_PLAYER_STATUS>(ec);
                if(!ec)
                {
                    m_callbacks[AWEMSG_PLAYER_STATUS](std::move(msg));
                }
            }
            break;
        }
    }
}
