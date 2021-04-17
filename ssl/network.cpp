#include "network.h"

#include "../network/network-impl.hpp"
#include "../network/reader-impl.hpp"
#include "../network/writer-impl.hpp"
#include "../network/ws-connector-impl.hpp"

template<>
void Endpoint<Config_SSL>::reset_socket()
{
    socket.reset();
}

template<>
void Endpoint<Config_WSS>::reset_socket()
{
    socket.~WSS_Socket();
    new (&socket) WSS_Socket(ioc);
}

template class Endpoint<Config_SSL>;
template class Endpoint<Config_WSS>;

template class SocketReader<SSL_Socket>;
template class WebsocketReader<SSL_Socket>;

template class SocketWriter<SSL_Socket>;
template class WebsocketWriter<SSL_Socket>;

template class WS_ClientConnector<WSS_Socket,SSL_ClientConnector>;
template class WS_ServerConnector<WSS_Socket,SSL_ServerConnector>;
