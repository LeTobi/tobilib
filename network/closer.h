#ifndef TC_NETWORK_CLOSER_H
#define TC_NETWORK_CLOSER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "../general/exception.hpp"

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
        using WSStream = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;
        
        WS_Closer(WSStream&, boost::asio::io_context&, Logger&);

        void request();
        void force();
        void cleanup();

    private:
        WSStream& socket;
        boost::asio::io_context& ioc;
        Logger& log;
        bool pending = false;

        void on_close(const boost::system::error_code&);
    };

} // namespace detail
} // namespace network
} // namespace tobilib

#endif