#include "websocket.h"
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include "errors.h"

#warning debug mode
#include <iostream>

using namespace tobilib;
using namespace network;
using namespace detail;
using boost::placeholders::_1;

template<class Stream, class StreamConnector>
WS_Client_Connect<Stream,StreamConnector>::WS_Client_Connect(
    const std::string& _address,
    unsigned int _port,
    Stream& _socket,
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

template<class Stream, class StreamConnector>
void WS_Client_Connect<Stream,StreamConnector>::tick()
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
        pending = true;
        socket.async_handshake(
            address,
            "/",
            boost::bind(&WS_Client_Connect<Stream,StreamConnector>::on_handshake,this,_1)
            );
    }
    if (hs_timer.due())
    {
        hs_timer.disable();
        reset();
        finished = true;
        error = boost::system::error_code(TobilibErrors.handshake_timeout,TobilibErrors);
    }
}

template<class Stream, class StreamConnector>
void WS_Client_Connect<Stream,StreamConnector>::connect()
{
    finished = false;
    error.clear();
    lowerlevel.connect();
}

template<class Stream, class StreamConnector>
void WS_Client_Connect<Stream,StreamConnector>::reset()
{
    lowerlevel.reset();
    socket.next_layer().close();
    hs_timer.disable();
    while (pending)
        ioc.poll_one();
    finished = false;
}

template<class Stream, class StreamConnector>
void WS_Client_Connect<Stream,StreamConnector>::on_handshake(const boost::system::error_code& ec)
{
    hs_timer.disable();
    pending = false;
    finished = true;
    error = ec;
}

template<class Stream, class StreamConnector>
WS_Server_Connect<Stream,StreamConnector>::WS_Server_Connect(
    Acceptor& _accpt,
    Stream& _socket,
    boost::asio::io_context& _ioc,
    ConnectorOptions& _options
    ):
        ioc(_ioc),
        socket(_socket),
        lowerlevel(_accpt,_socket.next_layer(),_ioc,_options),
        Connector(_options)
{ }

template<class Stream, class StreamConnector>
void WS_Server_Connect<Stream,StreamConnector>::tick()
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
        pending = true;
        std::cout << "handshake?" << std::endl; // TODO: Entfernen
        socket.async_accept(boost::bind(&WS_Server_Connect::on_handshake,this,_1));
        std::cout << "handshake!" << std::endl; // TODO: Entfernen
    }
    if (hs_timer.due())
    {
        hs_timer.disable();
        reset();
        finished = true;
        error = boost::system::error_code(TobilibErrors.handshake_timeout,TobilibErrors);
    }
}

template<class Stream, class StreamConnector>
void WS_Server_Connect<Stream,StreamConnector>::connect()
{
    finished = false;
    error.clear();
    lowerlevel.connect();
}

template<class Stream, class StreamConnector>
void WS_Server_Connect<Stream,StreamConnector>::reset()
{
    lowerlevel.reset();
    hs_timer.disable();
    while (pending)
        ioc.poll_one();
    finished = false;
}

template<class Stream, class StreamConnector>
void WS_Server_Connect<Stream,StreamConnector>::on_handshake(const boost::system::error_code& ec)
{
    hs_timer.disable();
    pending = false;
    finished = true;
    error = ec;
}

template class WS_Client_Connect<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>,TCP_Client_Connect>;
template class WS_Server_Connect<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>,Acceptor::Interface>;