#ifndef TC_WEBSOCKET_SERVERENDPOINT_H
#define TC_WEBSOCKET_SERVERENDPOINT_H

#include "../interface.h"
#include "endpoint.h"

namespace tobilib {
namespace network {

    class WS_Server_Endpoint: public Server_Endpoint, public WS_Endpoint
    {
    public:
        WS_Server_Endpoint(Acceptor&);

        void tick();
        void connect();

    private:
        Timer timeout;

        void on_handshake(const boost::system::error_code&);
    };

} // namespace network
} // namespace tobilib

#endif