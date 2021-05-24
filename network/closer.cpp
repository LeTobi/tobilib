#include "closer.h"
#include <boost/bind/bind.hpp>

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

TCP_Closer::TCP_Closer(
    TCP_Socket* _socket,
    boost::asio::io_context& _ioc
    ):
        socket(_socket),
        ioc(_ioc)
{ }

WS_Closer::WS_Closer(
    WS_Socket* _socket,
    boost::asio::io_context& _ioc
    ):
        socket(_socket),
        ioc(_ioc)
{ }

void TCP_Closer::request()
{
    boost::system::error_code ec;
    socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both,ec);
}

void WS_Closer::request()
{
    asio_closing = true;
    socket->async_close(
        boost::beast::websocket::close_reason("Shutdown by tobilib"),
        boost::bind(&WS_Closer::on_close,this,_1)
    );
}

void TCP_Closer::force()
{
    boost::system::error_code ec;
    socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket->close();
}

void WS_Closer::force()
{
    boost::system::error_code ec;
    socket->next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    socket->next_layer().close();
}

bool TCP_Closer::is_closing() const
{
    return false;
}

bool WS_Closer::is_closing() const
{
    return asio_closing;
}

void TCP_Closer::reset(TCP_Socket* sock)
{
    socket = sock;
    error.clear();
}

void WS_Closer::reset(WS_Socket* sock)
{
    socket = sock;
    error.clear();
}

void WS_Closer::on_close(const boost::system::error_code& err)
{
    asio_closing = false;
    error = err;
}
