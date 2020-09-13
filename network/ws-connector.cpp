#include "ws-connector.h"
#include <boost/bind/bind.hpp>
#include "errors.h"
#include "../general/exception.hpp"

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

template<class WebsocketType, class ConnectorType>
WS_ClientConnector<WebsocketType,ConnectorType>::WS_ClientConnector(
    const std::string& _address,
    unsigned int _port,
    WebsocketType& _socket,
    boost::asio::io_context& _ioc,
    ConnectorOptions& _options
    ):
        ioc(_ioc),
        socket(_socket),
        lowerlevel(_address,_port,_socket.next_layer(),_ioc,_options),
        address(_address),
        port(_port),
        Connector(_options)
{ }

template<class WebsocketType, class ConnectorType>
WS_ServerConnector<WebsocketType,ConnectorType>::WS_ServerConnector(
    Acceptor& _accpt,
    WebsocketType& _socket,
    boost::asio::io_context& _ioc,
    ConnectorOptions& _options
    ):
        ioc(_ioc),
        socket(_socket),
        lowerlevel(_accpt,_socket.next_layer(),_ioc,_options),
        Connector(_options)
{ }

template<class WebsocketType, class ConnectorType>
void WS_ClientConnector<WebsocketType,ConnectorType>::tick()
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
        socket.async_handshake(
            address,
            "/",
            boost::bind(&WS_ClientConnector<WebsocketType,ConnectorType>::on_handshake,this,_1)
            );
    }
    if (hs_timer.due())
    {
        hs_timer.disable();
        finished = true;
        error = boost::system::error_code(TobilibErrors.handshake_timeout,TobilibErrors);
    }
}

template<class WebsocketType, class ConnectorType>
void WS_ServerConnector<WebsocketType,ConnectorType>::tick()
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
        socket.async_accept(boost::bind(&WS_ServerConnector::on_handshake,this,_1));
    }
    if (hs_timer.due())
    {
        hs_timer.disable();
        finished = true;
        error = boost::system::error_code(TobilibErrors.handshake_timeout,TobilibErrors);
    }
}

template<class WebsocketType, class ConnectorType>
void WS_ClientConnector<WebsocketType,ConnectorType>::connect()
{
    finished = false;
    error.clear();
    lowerlevel.connect();
}

template<class WebsocketType, class ConnectorType>
void WS_ServerConnector<WebsocketType,ConnectorType>::connect()
{
    finished = false;
    error.clear();
    lowerlevel.connect();
}

template<class WebsocketType, class ConnectorType>
bool WS_ClientConnector<WebsocketType,ConnectorType>::is_async() const
{
    return async || lowerlevel.is_async();
}

template<class WebsocketType, class ConnectorType>
bool WS_ServerConnector<WebsocketType,ConnectorType>::is_async() const
{
    return async || lowerlevel.is_async();
}

template<class WebsocketType, class ConnectorType>
void WS_ClientConnector<WebsocketType,ConnectorType>::reset()
{
    if (is_async())
        throw Exception("reset mit ausstehender Operation","WS_ClientConnector::reset()");
    lowerlevel.reset();
    hs_timer.disable();
    finished = false;
}

template<class WebsocketType, class ConnectorType>
void WS_ServerConnector<WebsocketType,ConnectorType>::reset()
{
    if (is_async())
        throw Exception("reset mit ausstehender operation","WS_ServerConnector::reset()");
    lowerlevel.reset();
    hs_timer.disable();
    finished = false;
}

template<class WebsocketType, class ConnectorType>
void WS_ClientConnector<WebsocketType,ConnectorType>::on_handshake(const boost::system::error_code& ec)
{
    hs_timer.disable();
    async = false;
    finished = true;
    error = ec;
}

template<class WebsocketType, class ConnectorType>
void WS_ServerConnector<WebsocketType,ConnectorType>::on_handshake(const boost::system::error_code& ec)
{
    hs_timer.disable();
    async = false;
    finished = true;
    error = ec;
}

#ifndef TC_SSL_IMPL_ONLY

    template class WS_ClientConnector<WS_Socket,TCP_ClientConnector>;
    template class WS_ServerConnector<WS_Socket,TCP_ServerConnector>;

#else

    template class WS_ClientConnector<WSS_Socket,SSL_ClientConnector>;
    template class WS_ServerConnector<WSS_Socket,SSL_ServerConnector>;

#endif

