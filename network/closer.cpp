#include "closer.h"
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

using namespace tobilib;
using namespace network;
using namespace detail;
using boost::placeholders::_1;

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

void TCP_Closer::close()
{
    socket.close();
}

void WS_Closer::close()
{
    pending = true;
    socket.async_close(
        boost::beast::websocket::close_reason("Shutdown by tobilib"),
        boost::bind(&WS_Closer::on_close,this,_1)
    );
}

void TCP_Closer::reset()
{
    socket.close();
}

void WS_Closer::reset()
{
    socket.next_layer().close();
    while (pending)
        ioc.poll_one();
}

void WS_Closer::on_close(const boost::system::error_code& err)
{
    pending = false;
    if (err)
    {
        log << "Fehler beim Schliessen: " << err.message() << std::endl;
        socket.next_layer().close();
    }
}