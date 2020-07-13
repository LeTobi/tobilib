#ifndef TC_NETWORK_WSCLIENT
#define TC_NETWORK_WSCLIENT

#include "endpoint.h"

namespace tobilib {
namespace network {

    class WS_Client_Endpoint: public Client_Endpoint, public WS_Endpoint
    {
    public:
        WS_Client_Endpoint(const std::string&, unsigned int);

        void tick();
        void connect();
        void abort();
        void reset();

    private:
        Timer timeout;
        bool handshaking = false;
        void on_handshake(const boost::system::error_code&);
    };

} // namespace network
} // namespace tobilib

#endif