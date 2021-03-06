#ifndef TC_NETWORK_ENDPOINT_H
#define TC_NETWORK_ENDPOINT_H

#include <string>
#include "../general/queue.hpp"
#include "../general/exception.hpp"
#include "reader.h"
#include "writer.h"
#include "closer.h"
#include "tcp-connector.h"
#include "ws-connector.h"
#include "acceptor.h"

namespace tobilib {
namespace network {

    struct EndpointOptions: public ReaderOptions, public WriterOptions, public ConnectorOptions
    {
    /* ConnectorOptions
        double handshake_timeout
       ReaderOptions
        double read_timeout
        double inactive_warning
       WriterOptions
        double send_timeout     */
        double connect_timeout = 0;
        double close_timeout = 0;
    };

    enum class EndpointStatus {
        connecting,
        connected,
        closing,
        closed
    };

    enum class EndpointEvent
    {
        connected,
        received,
        inactive,
        closed,
    };

    template<class StackConfig>
    class Endpoint
    {
    public:
        Endpoint(Acceptor&);
        Endpoint(const std::string&, unsigned int);
        Endpoint(const Endpoint&) = delete;
        ~Endpoint();


        Logger log = std::string("Network-Endpoint: ");
        EndpointStatus status() const;
        bool is_connected() const;
        bool is_closed() const;
        boost::asio::ip::address remote_ip() const;

        void tick();
        void connect();
        void write(const std::string&);
        std::string read(unsigned int=0);
        std::string peek(unsigned int=0) const;
        unsigned int recv_size() const;
        void close();
        void reset();
        
        Queue<EndpointEvent> events;
        EndpointOptions options;
    private:
        using ConnectorType = detail::Connector<typename StackConfig::Socket>;

        void set_connecting();
        void set_connected();
        void set_closing();
        void set_closed();

        void reset_socket();

        boost::asio::io_context       ioc;
        typename StackConfig::Socket  socket;
        EndpointStatus                _status    = EndpointStatus::closed;
        boost::asio::ip::address      _remote_ip;
        ConnectorType*                connector;
        typename StackConfig::Reader  reader;
        typename StackConfig::Writer  writer;
        typename StackConfig::Closer  closer;
        
        Timer                         connect_timer;
        Timer                         close_timer;
        
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ioc);
    };

namespace detail {

    struct Config_TCP
    {
        using Socket = TCP_Socket;
        using ServerConnector = TCP_ServerConnector;
        using ClientConnector = TCP_ClientConnector;
        using Reader = SocketReader<TCP_Socket>;
        using Writer = SocketWriter<TCP_Socket>;
        using Closer = TCP_Closer;
    };

    struct Config_WS
    {
        using Socket = WS_Socket;
        using ServerConnector = WS_ServerConnector<WS_Socket,TCP_ServerConnector>;
        using ClientConnector = WS_ClientConnector<WS_Socket,TCP_ClientConnector>;
        using Reader = WebsocketReader<TCP_Socket>;
        using Writer = WebsocketWriter<TCP_Socket>;
        using Closer = WS_Closer;
    };

} // namespace detail

    using TCP_Endpoint = Endpoint<detail::Config_TCP>;
    using WS_Endpoint = Endpoint<detail::Config_WS>;

} // namespace network
} // namespace tobilib

#endif