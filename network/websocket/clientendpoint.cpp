#include "clientendpoint.h"
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

using namespace tobilib;
using namespace network;
using boost::placeholders::_1;

WS_Client_Endpoint::WS_Client_Endpoint(const std::string& host, unsigned int port): Client_Endpoint(host,port)
{ }

void WS_Client_Endpoint::connect()
{
    if (status()!=Status::unused)
        throw Exception("Implementierungsfehler: connect() mit falschem status","WS_Server_Endpoint::connect()");
    set_connecting();
    if (options.connection_timeout>0)
        timeout.set(options.connection_timeout);
    tcp_connect(socket.next_layer());
}

void WS_Client_Endpoint::tick()
{
    client_endpoint_tick();
    ws_endpoint_tick();
    if (timeout.due())
    {
        timeout.disable();
        log << "verbindungstimeout" << std::endl;
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
        socket.async_handshake(_host,"/",boost::bind(&WS_Client_Endpoint::on_handshake,this,_1));
    }
}

void WS_Client_Endpoint::abort()
{
    client_endpoint_abort();
    ws_endpoint_abort();
    endpoint_abort();
    while (handshaking)
        ioc.poll_one();
    set_closed();
}

void WS_Client_Endpoint::reset()
{
    abort();
    client_endpoint_reset();
    ws_endpoint_reset();
    endpoint_reset();
}

void WS_Client_Endpoint::on_handshake(const boost::system::error_code& ec)
{
    handshaking = false;
    timeout.disable();
    if (ec)
    {
        fail("Handshake-Fehler: "+ec.message());
        return;
    }
    start_reading();
}