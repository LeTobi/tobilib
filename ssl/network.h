#include "../network/network.h"
#include "connector.h"
#include "closer.h"

namespace tobilib {
namespace network {
namespace detail {

    struct Config_SSL
    {
        using Stream = SSL_Stream;
        using ServerConnector = SSL_Server_Connect;
        using ClientConnector = SSL_Client_Connect;
        using Reader = StreamReader<SSL_Stream>;
        using Writer = StreamWriter<SSL_Stream>;
        using Closer = SSL_Closer;
    };

    struct Config_WSS
    {
        using Stream = boost::beast::websocket::stream<SSL_Stream>;
        using ServerConnector = WS_Server_Connect<Stream,SSL_Server_Connect>;
        using ClientConnector = WS_Client_Connect<Stream,SSL_Client_Connect>;
        using Reader = WS_Reader<SSL_Stream>;
        using Writer = WS_Writer<SSL_Stream>;
        using Closer = WSS_Closer;
    };

} // namespace detail

using SSL_Endpoint = Endpoint<detail::Config_SSL>;
using WSS_Endpoint = Endpoint<detail::Config_WSS>;

} // namespace network
} // namespace tobilib