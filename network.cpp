#include "network.hpp"
#include <boost/asio/impl/src.hpp>


namespace awe
{
    network::network()
        : m_service(1), m_acc(m_service), m_sock(m_service) {}

    network::~network() = default;

    void network::connect(
            const boost::asio::ip::address& addr,
            unsigned short port,
            boost::system::error_code& ec
    ) {
        namespace asio = boost::asio;

        boost::asio::ip::tcp::endpoint ep(addr, port);
        m_sock.connect(ep, ec);
    }
    void network::accept(
        unsigned short port,
        boost::system::error_code& ec
    ) {
        namespace asio = boost::asio;
        asio::ip::tcp::endpoint ep(asio::ip::address(), port);
        m_acc = asio::ip::tcp::acceptor(m_service, ep);
        m_acc.accept(m_sock, ec);
    }
}
