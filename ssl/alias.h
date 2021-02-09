#ifndef TC_NETWORK_SSL_ALIAS_H
#define TC_NETWORK_SSL_ALIAS_H

#include <boost/asio/ssl.hpp>
#include "../network/alias.h"

namespace tobilib {
namespace network {

    extern boost::asio::ssl::context ssl_client_ctx;
    extern boost::asio::ssl::context ssl_server_ctx;

namespace detail {

    using SSL_Socket_Asio = boost::asio::ssl::stream<TCP_Socket>;
    
    class SSL_Socket: public SSL_Socket_Asio
    {
    public:
        SSL_Socket(boost::asio::io_context&);

        void reassign(int,const std::string&);

        int role;
        std::string sni;
        boost::asio::io_context& ioc;
    };

    using WSS_Socket = boost::beast::websocket::stream<SSL_Socket>;

} // namespace detail
} // namespace network
} // namespace tobilib    

#endif