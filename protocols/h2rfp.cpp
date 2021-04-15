#include "h2rfp.h"

using namespace tobilib;
using namespace h2rfp;

#ifndef TC_SSL_IMPL_ONLY
    Response::Response():
        requested(false),
        received(false)
    { }

    Response::Response(unsigned int _id):
        requested(true),
        received(false),
        id(_id)
    { }

    bool Response::is_requested() const
    {
        return requested;
    }

    bool Response::is_received() const
    {
        return received;
    }

    bool Response::pull(ResponseList& list)
    {
        if (list.count(id)>0)
        {
            data = std::move(list.at(id));
            list.erase(id);
            received=true;
        }
        return received;
    }

    void Response::dismiss()
    {
        requested=false;
        received=false;
        id = 0;
    }
#endif

template<class NetworkEndpoint>
Endpoint<NetworkEndpoint>::Endpoint(network::Acceptor& accpt):
    network_endpoint(accpt),
    options(network_endpoint.options)
{
    parser.log.parent = &log;
    network_endpoint.log.parent = &log;
}

template<class NetworkEndpoint>
Endpoint<NetworkEndpoint>::Endpoint(const std::string& address, unsigned int port):
    network_endpoint(address, port),
    options(network_endpoint.options)
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
                ev.msg = std::move(parser.output.next());
                if (ev.msg.name.empty())
                {
                    responses[ev.msg.id] = std::move(ev.msg.data);
                }
                else
                {
                    ev.type = EventType::message;
                    events.push(ev);
                }
            }
        }   break;

        case network::EndpointEvent::inactive:{
            EndpointEvent ev;
            ev.type = EventType::inactive;
            events.push(ev);
        }   break;

        case network::EndpointEvent::closed:{
            EndpointEvent ev;
            ev.type = EventType::closed;
            events.push(ev);
            parser.reset();
        }   break;
    }
}

template<class NetworkEndpoint>
void Endpoint<NetworkEndpoint>::connect()
{
    network_endpoint.connect();
}

template<class NetworkEndpoint>
void Endpoint<NetworkEndpoint>::notify(const std::string& name, const JSObject& data)
{
    Message msg;
    msg.name = name;
    msg.data = data;
    msg.id = 0;
    network_endpoint.write(msg.to_string());
}

template<class NetworkEndpoint>
Response Endpoint<NetworkEndpoint>::request(const std::string& name, const JSObject& data)
{
    Message msg;
    msg.name = name;
    msg.data = data;
    msg.id = nextid;
    nextid = (nextid%10000)+1;
    network_endpoint.write(msg.to_string());
    return Response(msg.id);
}

template<class NetworkEndpoint>
void Endpoint<NetworkEndpoint>::respond(unsigned int id, const JSObject& data)
{
    Message msg;
    msg.data = data;
    msg.id = id;
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
    responses.clear();
}

#ifdef TC_SSL_IMPL_ONLY
    template class Endpoint<network::SSL_Endpoint>;
    template class Endpoint<network::WSS_Endpoint>;
#else
    template class Endpoint<network::TCP_Endpoint>;
    template class Endpoint<network::WS_Endpoint>;
#endif