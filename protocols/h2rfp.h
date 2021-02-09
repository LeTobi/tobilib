#ifndef TC_PROTOCOLS_H2RFP
#define TC_PROTOCOLS_H2RFP

#include "./parser/ph2rfp.h"
#include "../network/network.h"

namespace tobilib {
namespace h2rfp {

    using EndpointStatus = network::EndpointStatus;
    using EndpointOptions = network::EndpointOptions;
    
    enum class EventType {
        request,
        callback,
        connected,
        inactive,
        closed
    };

    class EndpointEvent {
    public:
        EventType type;
        Message msg;
    };

    template<class NetworkEndpoint>
    class Endpoint
    {
    public:
        Endpoint(network::Acceptor&);
        Endpoint(const std::string&, unsigned int);
        Endpoint(const Endpoint&) = delete;

        Logger log = std::string("h2rfp-Endpoint: ");
        EndpointStatus status() const;
        bool is_connected() const;
        bool is_closed() const;
        boost::asio::ip::address remote_ip() const;

        void tick();
        void connect();
        void send(const Message&);
        void close();
        void reset();

        Queue<EndpointEvent> events;
        EndpointOptions options;

    private:
        detail::Parser parser;
        NetworkEndpoint network_endpoint;
    };

    using TCP_Endpoint = Endpoint<network::TCP_Endpoint>;
    using WS_Endpoint = Endpoint<network::WS_Endpoint>;

} // namespace h2rfp
} // namespace tobilib

#endif