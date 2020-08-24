#include "closer.h"
#include <boost/bind/bind.hpp>

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

SSL_Closer::SSL_Closer(SSL_Stream& _socket, boost::asio::io_context& _ioc, Logger& _log):
    socket(_socket),
    ioc(_ioc),
    log(_log)
{ }

WSS_Closer::WSS_Closer(WSStream& _socket, boost::asio::io_context& _ioc, Logger& _log):
    socket(_socket),
    ioc(_ioc),
    log(_log)
{ }

void SSL_Closer::request()
{
    pending = true;
    socket.async_shutdown(boost::bind(&SSL_Closer::on_close,this,_1));
}

void WSS_Closer::request()
{
    pending = true;
    socket.async_close(
        boost::beast::websocket::close_reason("Shutdown by tobilib"),
        boost::bind(&WSS_Closer::on_close,this,_1)
    );
}

void SSL_Closer::force()
{
    boost::system::error_code ec;
    socket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    while (pending)
        ioc.poll_one();
}

void WSS_Closer::force()
{
    boost::system::error_code ec;
    socket.next_layer().next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    while (pending)
        ioc.poll_one();
}

void SSL_Closer::cleanup()
{
    socket.reassign(socket.role,socket.sni);
}

void WSS_Closer::cleanup()
{
    socket.next_layer().reassign(
        socket.next_layer().role,
        socket.next_layer().sni
        );
}

void SSL_Closer::on_close(const boost::system::error_code& err)
{
    pending = false;
    if (err)
    {
        log << "Fehler beim Schliessen: " << err.message() << std::endl;
        force();
    }
}

void WSS_Closer::on_close(const boost::system::error_code& err)
{
    pending = false;
    if (err)
    {
        log << "Fehler beim Schliessen: " << err.message() << std::endl;
        force();
    }
}
