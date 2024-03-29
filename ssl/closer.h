#ifndef TC_NETWORK_SSL_CLOSER_H
#define TC_NETWORK_SSL_CLOSER_H

#include <boost/beast.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include "alias.h"
#include "../general/exception.hpp"


namespace boost {
namespace beast {
namespace websocket {
    template<class TeardownHandler>
    void async_teardown(
        boost::beast::role_type role,
        tobilib::network::detail::SSL_Socket& stream,
        TeardownHandler&& handler)
    {
        tobilib::network::detail::SSL_Socket_Asio& nextlayer = stream;
        async_teardown(
            role,
            nextlayer,
            std::forward<TeardownHandler>(handler)
        );
    }
} // namespace websocket
} // namespace beast
} // namespace boost

namespace tobilib {
namespace network {
namespace detail {

    class SSL_Closer
    {
    public:
        SSL_Closer(SSL_Socket*, boost::asio::io_context&);

        void request();
        void force();
        bool is_closing() const;
        void reset(SSL_Socket*);

        boost::system::error_code error;

    private:
        SSL_Socket* socket;
        boost::asio::io_context& ioc;
        bool asio_closing = false;

        void on_close(const boost::system::error_code&);
    };

    class WSS_Closer
    {
    public:
        WSS_Closer(WSS_Socket*, boost::asio::io_context&);

        void request();
        void force();
        bool is_closing() const;
        void reset(WSS_Socket*);

        boost::system::error_code error;

    private:
        WSS_Socket* socket;
        boost::asio::io_context& ioc;
        bool asio_closing = false;

        void on_close(const boost::system::error_code&);
    };

} // namespace detail
} // namespace network
} // namespace tobilib

#endif