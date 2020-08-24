#ifndef TC_NETWORK_ENDPOINT_H
#define TC_NETWORK_ENDPOINT_H

#include <string>
#include <boost/asio.hpp>
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
        void set_connecting();
        void set_connected();
        void set_closing();
        void set_closed();

        boost::asio::io_context      ioc;
        typename StackConfig::Stream socket;
        EndpointStatus               _status    = EndpointStatus::closed;
        boost::asio::ip::address     _remote_ip;
        detail::Connector*           connector;
        typename StackConfig::Reader reader;
        typename StackConfig::Writer writer;
        typename StackConfig::Closer closer;
        
        Timer                    connect_timer;
        Timer                    close_timer;
        
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ioc);
    };

namespace detail {

    struct Config_TCP
    {
        using Stream = boost::asio::ip::tcp::socket;
        using ServerConnector = TCP_Server_Connect;
        using ClientConnector = TCP_Client_Connect;
        using Reader = StreamReader<boost::asio::ip::tcp::socket>;
        using Writer = StreamWriter<boost::asio::ip::tcp::socket>;
        using Closer = TCP_Closer;
    };

    struct Config_WS
    {
        using Stream = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;
        using ServerConnector = WS_Server_Connect<Stream,TCP_Server_Connect>;
        using ClientConnector = WS_Client_Connect<Stream,TCP_Client_Connect>;
        using Reader = WS_Reader<boost::asio::ip::tcp::socket>;
        using Writer = WS_Writer<boost::asio::ip::tcp::socket>;
        using Closer = WS_Closer;
    };

} // namespace detail

    using TCP_Endpoint = Endpoint<detail::Config_TCP>;
    using WS_Endpoint = Endpoint<detail::Config_WS>;

} // namespace network
} // namespace tobilib

#endif