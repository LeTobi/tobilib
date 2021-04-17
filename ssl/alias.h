#ifndef TC_NETWORK_SSL_ALIAS_H
#define TC_NETWORK_SSL_ALIAS_H

#include <boost/asio/ssl.hpp>
#include "../network/alias.h"

namespace tobilib {
namespace network {
namespace detail {

    using SSL_Socket_Asio = boost::asio::ssl::stream<TCP_Socket>;
    
    class SSL_Socket: public SSL_Socket_Asio
    {
    public:
        SSL_Socket(boost::asio::io_context&);
        ~SSL_Socket();

        void set_client(const std::string&);
        void set_server();
        void reset();

    private:
        void clear();
        void fill_server();
        void fill_client();

        boost::asio::ssl::context* ssl_ctx;
        int role;
        std::string sni;
        boost::asio::io_context& ioc;
    };

    using WSS_Socket = boost::beast::websocket::stream<SSL_Socket>;

} // namespace detail
} // namespace network
} // namespace tobilib    

#endif