#include "connector.h"
#include <boost/bind/bind.hpp>
#include "../network/errors.h"
#include "../general/exception.hpp"

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

SSL_ClientConnector::SSL_ClientConnector(
    const std::string& address,
    unsigned int port,
    SSL_Socket* _socket,
    boost::asio::io_context& ioc,
    ConnectorOptions& _options):
        Connector(_options),
        socket(_socket),
        lowerlevel(address,port,&_socket->next_layer(),ioc,_options)
{
    socket->reassign(SSL_Socket_Asio::handshake_type::client,address);
}

SSL_ServerConnector::SSL_ServerConnector(
    Acceptor& accpt,
    SSL_Socket* _socket,
    boost::asio::io_context& ioc,
    ConnectorOptions& _options):
        Connector(_options),
        socket(_socket),
        lowerlevel(accpt,&_socket->next_layer(),ioc,_options)
{
    socket->reassign(SSL_Socket_Asio::handshake_type::server,"");
}

void SSL_ClientConnector::tick()
{
    lowerlevel.tick();
    if (lowerlevel.finished)
    {
        lowerlevel.finished = false;
        if (lowerlevel.error)
        {
            error = lowerlevel.error;
            finished = true;
            return;
        }
        if (options.handshake_timeout>0)
            hs_timer.set(options.handshake_timeout);
        remote_ip = lowerlevel.remote_ip;
        async = true;
        socket->async_handshake(
                SSL_Socket_Asio::handshake_type::client,
                boost::bind(&SSL_ClientConnector::on_handshake,this,_1)
            );
    }
    if (hs_timer.due())
    {
        hs_timer.disable();
        finished = true;
        error = boost::system::error_code(TobilibErrors.handshake_timeout,TobilibErrors);
    }
}

void SSL_ServerConnector::tick()
{
    lowerlevel.tick();
    if (lowerlevel.finished)
    {
        lowerlevel.finished = false;
        if (lowerlevel.error)
        {
            error = lowerlevel.error;
            finished = true;
            return;
        }
        if (options.handshake_timeout>0)
            hs_timer.set(options.handshake_timeout);
        remote_ip = lowerlevel.remote_ip;
        async = true;
        socket->async_handshake(
                SSL_Socket_Asio::handshake_type::server,
                boost::bind(&SSL_ServerConnector::on_handshake,this,_1)
            );
    }
    if (hs_timer.due())
    {
        hs_timer.disable();
        finished = true;
        error = boost::system::error_code(TobilibErrors.handshake_timeout,TobilibErrors);
    }
}

void SSL_ClientConnector::connect()
{
    finished = false;
    error.clear();
    lowerlevel.connect();
}

void SSL_ServerConnector::connect()
{
    finished = false;
    error.clear();
    lowerlevel.connect();
}

bool SSL_ClientConnector::is_async() const
{
    return async || lowerlevel.is_async();
}

bool SSL_ServerConnector::is_async() const
{
    return async || lowerlevel.is_async();
}

void SSL_ClientConnector::cancel()
{
    lowerlevel.cancel();
}

void SSL_ServerConnector::cancel()
{
    lowerlevel.cancel();
}

void SSL_ClientConnector::reset(SSL_Socket* sock)
{
    if (is_async())
        throw Exception("reset mit ausstehender Operation","SSL_ClientConnector::reset()");
    socket = sock;
    hs_timer.disable();
    lowerlevel.reset(&socket->next_layer());
    finished = false;
}

void SSL_ServerConnector::reset(SSL_Socket* sock)
{
    if (is_async())
        throw Exception("reset mit ausstehender Operation","SSL_ServerConnector::reset()");
    socket = sock;
    hs_timer.disable();
    lowerlevel.reset(&socket->next_layer());
    finished = false;
}

void SSL_ClientConnector::on_handshake(const boost::system::error_code& ec)
{
    hs_timer.disable();
    async = false;
    finished = true;
    error = ec;
}

void SSL_ServerConnector::on_handshake(const boost::system::error_code& ec)
{
    hs_timer.disable();
    async = false;
    finished = true;
    error = ec;
}