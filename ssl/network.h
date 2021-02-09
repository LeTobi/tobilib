#ifndef TC_NETWORK_SSL_H
#define TC_NETWORK_SSL_H

#include "alias.h"
#include "../network/network.h"
#include "connector.h"
#include "closer.h"

namespace tobilib {
namespace network {

    void ssl_server_init(const std::string&);

namespace detail {

    struct Config_SSL
    {
        using Socket = SSL_Socket;
        using ServerConnector = SSL_ServerConnector;
        using ClientConnector = SSL_ClientConnector;
        using Reader = SocketReader<SSL_Socket>;
        using Writer = SocketWriter<SSL_Socket>;
        using Closer = SSL_Closer;
    };

    struct Config_WSS
    {
        using Socket = WSS_Socket;
        using ServerConnector = WS_ServerConnector<WSS_Socket,SSL_ServerConnector>;
        using ClientConnector = WS_ClientConnector<WSS_Socket,SSL_ClientConnector>;
        using Reader = WebsocketReader<SSL_Socket>;
        using Writer = WebsocketWriter<SSL_Socket>;
        using Closer = WSS_Closer;
    };

} // namespace detail

using SSL_Endpoint = Endpoint<detail::Config_SSL>;
using WSS_Endpoint = Endpoint<detail::Config_WSS>;

} // namespace network
} // namespace tobilib

#endif