#ifndef TC_NETWORK_SSL_H
#define TC_NETWORK_SSL_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "../network/tcp-connector.h"
#include "../network/acceptor.h"
#include <memory>
#include <string>

namespace tobilib {
namespace network {

extern boost::asio::ssl::context ssl_client_ctx;
extern boost::asio::ssl::context ssl_server_ctx;
void ssl_server_init(const std::string&);

namespace detail {

using Asio_SSL_Stream = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

class SSL_Stream: public Asio_SSL_Stream
{
public:
    SSL_Stream(boost::asio::io_context&);

    void reassign(int,const std::string&);

    int role;
    std::string sni;
    boost::asio::io_context& ioc;
};

class SSL_Client_Connect: public Connector
{
public:
    SSL_Client_Connect(const std::string&, unsigned int, SSL_Stream&, boost::asio::io_context&, ConnectorOptions&);

    void tick();
    void connect();
    bool is_async() const;
    void reset();

private:
    bool async = false;
    Timer hs_timer;
    TCP_Client_Connect lowerlevel;
    SSL_Stream& socket;

    void on_handshake(const boost::system::error_code&);
};

class SSL_Server_Connect: public Connector
{
public:
    SSL_Server_Connect(Acceptor&, SSL_Stream&, boost::asio::io_context&, ConnectorOptions&);

    void tick();
    void connect();
    bool is_async() const;
    void reset();

private:
    bool async = false;
    Timer hs_timer;
    TCP_Server_Connect lowerlevel;
    SSL_Stream& socket;

    void on_handshake(const boost::system::error_code&);
};

} // namespace detail
} // namespace network
} // namespace tobilib

#endif