#include "network.hpp"
#include <boost/asio/impl/src.hpp>


namespace awe
{
    network::network()
        : m_service(1), m_acc(m_service), m_sock(m_service) {}

    network::~network() = default;

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
            m_role = ROLE_CLIENT;
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
            m_role = ROLE_SERVER;
    }

    void network::reset()
    {
        m_role = ROLE_NONE;
        m_sock.close();
        m_acc.close();
        m_service.reset();
    }
}
