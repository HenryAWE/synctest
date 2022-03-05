#pragma once

#include <future>
#ifdef _WIN32
#   include <sdkddkver.h>
#endif
#include <boost/asio.hpp>


namespace awe
{
    class network
    {
    public:
        network();

        virtual ~network();

        boost::asio::ip::tcp::socket& get_socket() { return m_sock; }

        void write(int data, boost::system::error_code ec);
        void write(std::uint64_t data, boost::system::error_code ec);

        template <typename T>
        void read(boost::system::error_code& ec) = delete;

        void connect(
            const boost::asio::ip::address& addr,
            unsigned short port,
            boost::system::error_code& ec
        );
        void accept(
            unsigned short port,
            boost::system::error_code& ec
        );
        
    protected:
        boost::asio::io_service m_service;
        boost::asio::ip::tcp::acceptor m_acc;
        boost::asio::ip::tcp::socket m_sock;
    };
}
