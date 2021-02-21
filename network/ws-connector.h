#ifndef TC_NETWORK_WEBSOCKET_H
#define TC_NETWORK_WEBSOCKET_H

#include "alias.h"
#include "tcp-connector.h"
#include "acceptor.h"
#include "../general/timer.hpp"

namespace tobilib {
namespace network {
namespace detail {

    template<class WebsocketType, class ConnectorType>
    class WS_ClientConnector: public Connector<WebsocketType>
    {
    public:
        WS_ClientConnector(const std::string&, unsigned int, WebsocketType*, boost::asio::io_context&, ConnectorOptions&);

        void tick();
        void connect();
        void cancel();
        void reset(WebsocketType*);

    private:
        std::string address;
        unsigned int port;
        ConnectorType lowerlevel;
        WebsocketType* socket;
        boost::asio::io_context& ioc;
        bool asio_handshaking = false;
        Timer hs_timer;

        void on_handshake(const boost::system::error_code&);
    };

    template<class WebsocketType, class ConnectorType>
    class WS_ServerConnector: public Connector<WebsocketType>
    {
    public:
        WS_ServerConnector(Acceptor&, WebsocketType*, boost::asio::io_context&, ConnectorOptions&);

        void tick();
        void connect();
        void cancel();
        void reset(WebsocketType*);

    private:
        ConnectorType lowerlevel;
        WebsocketType* socket;
        boost::asio::io_context& ioc;
        bool asio_handshaking = false;
        Timer hs_timer;

        void on_handshake(const boost::system::error_code&);
    };

} // namespace detail
} // namespace network
} // namespace tobilib

#endif