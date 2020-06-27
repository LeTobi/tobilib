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
        log << "verbindungstimeout" << std::endl;
        abort();
        return;
    }
    if (tcp_connected)
    {
        tcp_connected = false;
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

void WS_Server_Endpoint::on_handshake(const boost::system::error_code& ec)
{
    if (ec)
    {
        fail("Handshake error");
        return;
    }
    start_reading();
}