#ifndef TC_PROTOCOLS_H2RFP
#define TC_PROTOCOLS_H2RFP

#include "./parser/ph2rfp.h"
#include "../network/network.h"
#include <map>

namespace tobilib {
namespace h2rfp {

    using EndpointStatus = network::EndpointStatus;
    using EndpointOptions = network::EndpointOptions;
    
    enum class EventType {
        connected,
        message,
        inactive,
        closed
    };

    class EndpointEvent {
    public:
        EventType type;
        Message msg;
    };

    using ResponseList = std::map<unsigned int,JSObject>;

    class Response
    {
    public:
        Response();
        Response(unsigned int);

        bool is_requested() const;
        bool is_received() const;
        void dismiss();
        bool pull(ResponseList&);

        JSObject data;
    
    private:
        bool requested;
        bool received;
        unsigned int id;
    };

    template<class NetworkEndpoint>
    class Endpoint
    {
    private:
        unsigned nextid = 1;

        detail::Parser parser;
        NetworkEndpoint network_endpoint;

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
        void notify(const std::string&, const JSObject& = JSObject());
        Response request(const std::string&, const JSObject& = JSObject());
        void respond(unsigned int id, const JSObject& = JSObject());
        void close();
        void reset();

        ResponseList responses;
        Queue<EndpointEvent> events;
        EndpointOptions& options;
    };

    using TCP_Endpoint = Endpoint<network::TCP_Endpoint>;
    using WS_Endpoint = Endpoint<network::WS_Endpoint>;

} // namespace h2rfp
} // namespace tobilib

#endif