#ifndef TC_NETWORK_CLOSER_H
#define TC_NETWORK_CLOSER_H

#include "../general/exception.hpp"
#include "alias.h"

namespace tobilib{
namespace network{
namespace detail {
    class TCP_Closer {
    public:
        TCP_Closer(boost::asio::ip::tcp::socket&,boost::asio::io_context&,Logger&);

        void request();
        void force();
        void cleanup();

    private:
        Logger& log;
        boost::asio::io_context& ioc;
        boost::asio::ip::tcp::socket& socket;
    };

    class WS_Closer
    {
    public:
        WS_Closer(WS_Socket&, boost::asio::io_context&, Logger&);

        void request();
        void force();
        void cleanup();

    private:
        WS_Socket& socket;
        boost::asio::io_context& ioc;
        Logger& log;
        bool pending = false;

        void on_close(const boost::system::error_code&);
    };

} // namespace detail
} // namespace network
} // namespace tobilib

#endif