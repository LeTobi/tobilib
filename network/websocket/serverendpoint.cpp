#include "serverendpoint.h"
#include <boost/bind.hpp>

using namespace tobilib;
using namespace network;

WS_Server_Endpoint::WS_Server_Endpoint(Acceptor& acceptor): Server_Endpoint(acceptor)
{ }

void WS_Server_Endpoint::tick()
{
    ws_endpoint_tick();
    server_endpoint_tick();
    if (timeout.due())
    {
        timeout.disable();
        log << "handshake-timeout" << std::endl;
        abort();
        return;
    }
    if (tcp_connected)
    {
        tcp_connected = false;
        if (tcp_result)
        {
            fail(tcp_result.message());
            return;
        }
        handshaking = true;
        if (options.connection_timeout>0)
            timeout.set(options.connection_timeout);
        socket.async_accept(boost::bind(&WS_Server_Endpoint::on_handshake,this,_1));
    }
}

void WS_Server_Endpoint::connect()
{
    if (status()!=Status::unused)
        throw Exception("Implementierungsfehler: connect() mit falschem status","WS_Server_Endpoint::connect()");
    set_connecting();
    tcp_connect(socket.next_layer());
}

void WS_Server_Endpoint::abort()
{
    server_endpoint_abort();
    ws_endpoint_abort();
    endpoint_abort();
    while (handshaking)
        ioc.poll_one();
    set_closed();
}

void WS_Server_Endpoint::reset()
{
    abort();
    server_endpoint_reset();
    ws_endpoint_reset();
    endpoint_reset();
}

void WS_Server_Endpoint::on_handshake(const boost::system::error_code& ec)
{
    handshaking = false;
    timeout.disable();
    if (ec)
    {
        fail("WS-Handshake error");
        return;
    }
    start_reading();
}