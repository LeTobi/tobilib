#include "closer.h"
#include <boost/bind/bind.hpp>

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

TCP_Closer::TCP_Closer(
    boost::asio::ip::tcp::socket& _socket,
    boost::asio::io_context& _ioc,
    Logger& _log
    ):
        socket(_socket),
        ioc(_ioc),
        log(_log)
{ }

WS_Closer::WS_Closer(
    WSStream& _socket,
    boost::asio::io_context& _ioc,
    Logger& _log
    ):
        socket(_socket),
        ioc(_ioc),
        log(_log)
{ }

void TCP_Closer::request()
{
    boost::system::error_code ec;
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,ec);
}

void WS_Closer::request()
{
    pending = true;
    socket.async_close(
        boost::beast::websocket::close_reason("Shutdown by tobilib"),
        boost::bind(&WS_Closer::on_close,this,_1)
    );
}

void TCP_Closer::force()
{
    socket.close();
}

void WS_Closer::force()
{
    boost::system::error_code ec;
    socket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    while (pending)
        ioc.poll_one();
}

void TCP_Closer::cleanup()
{
    socket.close();
}

void WS_Closer::cleanup()
{
    socket.~WSStream();
    new (&socket) WSStream(ioc);
}

void WS_Closer::on_close(const boost::system::error_code& err)
{
    pending = false;
    if (err)
    {
        log << "Fehler beim Schliessen: " << err.message() << std::endl;
        force();
    }
}
