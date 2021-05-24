#include "network-impl.hpp"

template<>
void Endpoint<Config_TCP>::reset_socket()
{
    socket.close();
}

template<>
void Endpoint<Config_WS>::reset_socket()
{
    socket.~WS_Socket();
    new (&socket) WS_Socket(ioc);
}

template class Endpoint<Config_TCP>;
template class Endpoint<Config_WS>;