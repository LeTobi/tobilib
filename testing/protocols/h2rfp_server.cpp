#include "../../protocols/h2rfp.h"
#include <iostream>

using namespace tobilib;
using namespace h2rfp;

network::Acceptor acceptor (15432);
TCP_Endpoint endpoint (acceptor);

Message stringRequest()
{
    h2rfp::Message out;
    out.name = "getString";
    out.id = 1;
    return out;
}

Message numberRequest()
{
    h2rfp::Message out;
    out.name = "getNumber";
    out.id = 2;
    return out;
}

void read_string(const JSObject& data)
{
    std::string value = data.get("value","");
    std::cout << "client sent string: " << value << std::endl;
}

void read_int(const JSObject& data)
{
    int value = data.get("value",0);
    std::cout << "client sent number: " << value << std::endl;
}

void react(const EndpointEvent& ev)
{
    switch (ev.type)
    {
    case EventType::connected:
        std::cout << "Connected: " << endpoint.remote_ip().to_string() << std::endl;
        endpoint.send(stringRequest());
        endpoint.send(numberRequest());
        break;
    case EventType::callback:
        if (ev.msg.id==1)
            read_string(ev.msg.data);
        else if (ev.msg.id==2)
            read_int(ev.msg.data);
        else
            std::cout << "unexpected callback from client (id " << ev.msg.id << ")" << std::endl;
        break;
    case EventType::request:
        if (ev.msg.name == "getName")
        {
            Message response;
            response.id = ev.msg.id;
            response.data.put("name","My Testserver");
            endpoint.send(response);
        }
        else
        {
            std::cout << "a request from the client arrived: " << ev.msg.name << std::endl;
        }
        break;
    case EventType::inactive:
        std::cout << "remote endpoint seems inactive" << std::endl;
        break;
    case EventType::closed:
        std::cout << "disconnected" << std::endl;
        endpoint.connect();
        break;
    }
}

int main()
{
    std::cout << "H2RFP testserver on Port 15432" << std::endl;
    endpoint.connect();
    while (true)
    {
        endpoint.tick();
        while (!endpoint.events.empty())
            react(endpoint.events.next());
    }
}