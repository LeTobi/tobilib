#ifndef TC_NETWORK_WEBSOCKET_H
#define TC_NETWORK_WEBSOCKET_H

#include "tcp.h"
#include "acceptor.h"
#include <boost/beast.hpp>
#include "../general/timer.hpp"

namespace tobilib {
namespace network {


namespace detail {

    template<class Stream, class StreamConnector>
    class WS_Client_Connect: public Connector
    {
    public:
        WS_Client_Connect(const std::string&, unsigned int, Stream&, boost::asio::io_context&, ConnectorOptions&);

        void tick();
        void connect();
        void reset();

    private:
        std::string address;
        unsigned int port;
        StreamConnector lowerlevel;
        Stream& socket;
        boost::asio::io_context& ioc;
        bool pending = false;
        Timer hs_timer;

        void on_handshake(const boost::system::error_code&);
    };

    template<class Stream, class StreamConnector>
    class WS_Server_Connect: public Connector
    {
    public:
        WS_Server_Connect(Acceptor&, Stream&, boost::asio::io_context&, ConnectorOptions&);

        void tick();
        void connect();
        void reset();

    private:
        StreamConnector lowerlevel;
        Stream& socket;
        boost::asio::io_context& ioc;
        bool pending = false;
        Timer hs_timer;

        void on_handshake(const boost::system::error_code&);
    };

} // namespace detail
} // namespace network
} // namespace tobilib

#endif