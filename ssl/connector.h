#ifndef TC_NETWORK_SSL_CONNECTOR_H
#define TC_NETWORK_SSL_CONNECTOR_H

#include <boost/asio.hpp>

#include <memory>
#include <string>

#include "alias.h"
#include "../network/tcp-connector.h"
#include "../network/acceptor.h"


namespace tobilib {
namespace network {
namespace detail {

class SSL_ClientConnector: public Connector<SSL_Socket>
{
public:
    SSL_ClientConnector(const std::string&, unsigned int, SSL_Socket*, boost::asio::io_context&, ConnectorOptions&);

    void tick();
    void connect();
    bool is_async() const;
    void cancel();
    void reset(SSL_Socket*);

private:
    bool async = false;
    Timer hs_timer;
    TCP_ClientConnector lowerlevel;
    SSL_Socket* socket;

    void on_handshake(const boost::system::error_code&);
};

class SSL_ServerConnector: public Connector<SSL_Socket>
{
public:
    SSL_ServerConnector(Acceptor&, SSL_Socket*, boost::asio::io_context&, ConnectorOptions&);

    void tick();
    void connect();
    bool is_async() const;
    void cancel();
    void reset(SSL_Socket*);

private:
    bool async = false;
    Timer hs_timer;
    TCP_ServerConnector lowerlevel;
    SSL_Socket* socket;

    void on_handshake(const boost::system::error_code&);
};

} // namespace detail
} // namespace network
} // namespace tobilib

#endif