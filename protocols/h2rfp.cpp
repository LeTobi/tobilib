#include "h2rfp.h"

using namespace tobilib;
using namespace h2rfp;

template<class NetworkEndpoint>
Endpoint<NetworkEndpoint>::Endpoint(network::Acceptor& accpt):
    network_endpoint(accpt)
{
    parser.log.parent = &log;
    network_endpoint.log.parent = &log;
}

template<class NetworkEndpoint>
Endpoint<NetworkEndpoint>::Endpoint(const std::string& address, unsigned int port):
    network_endpoint(address, port)
{
    parser.log.parent = &log;
    network_endpoint.log.parent = &log;
}

template<class NetworkEndpoint>
EndpointStatus Endpoint<NetworkEndpoint>::status() const
{
    return network_endpoint.status();
}

template<class NetworkEndpoint>
bool Endpoint<NetworkEndpoint>::is_connected() const
{
    return network_endpoint.is_connected();
}

template<class NetworkEndpoint>
bool Endpoint<NetworkEndpoint>::is_closed() const
{
    return network_endpoint.is_closed();
}

template<class NetworkEndpoint>
boost::asio::ip::address Endpoint<NetworkEndpoint>::remote_ip() const
{
    return network_endpoint.remote_ip();
}

template<class NetworkEndpoint>
void Endpoint<NetworkEndpoint>::tick()
{
    network_endpoint.tick();

    while (!network_endpoint.events.empty())
    switch(network_endpoint.events.next())
    {
        case network::EndpointEvent::connected: {
            EndpointEvent ev;
            ev.type = EventType::connected;
            events.push(ev);
        }break;

        case network::EndpointEvent::received: {
            parser.feed(network_endpoint.read());
            if (!parser.is_good())
            {
                network_endpoint.close();
                break;
            }
            while (!parser.output.empty())
            {
                EndpointEvent ev;
                ev.msg = parser.output.next();
                ev.type = ev.msg.name.empty()?
                    EventType::callback:
                    EventType::request;
                events.push(ev);
            }
        }break;

        case network::EndpointEvent::inactive:{
            EndpointEvent ev;
            ev.type = EventType::inactive;
            events.push(ev);
        }break;
        case network::EndpointEvent::closed:{
            EndpointEvent ev;
            ev.type = EventType::closed;
            events.push(ev);
            parser.reset();
        }break;
    }
}

template<class NetworkEndpoint>
void Endpoint<NetworkEndpoint>::connect()
{
    network_endpoint.connect();
}

template<class NetworkEndpoint>
void Endpoint<NetworkEndpoint>::send(const Message& msg)
{
    network_endpoint.write(msg.to_string());
}

template<class NetworkEndpoint>
void Endpoint<NetworkEndpoint>::close()
{
    network_endpoint.close();
}

template<class NetworkEndpoint>
void Endpoint<NetworkEndpoint>::reset()
{
    network_endpoint.reset();
    parser.reset();
    events.clear();
}

template class Endpoint<network::TCP_Endpoint>;
template class Endpoint<network::WS_Endpoint>;