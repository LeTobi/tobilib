#include "../../protocols/h2rfp.h"
#include <iostream>

using namespace tobilib;
using namespace h2rfp;

network::Acceptor acceptor (15432);
TCP_Endpoint endpoint (acceptor);
Response stringmsg;
Response intmsg;

void read_string()
{
    std::string value = stringmsg.data.get("value","");
    std::cout << "client sent string: " << value << std::endl;
    stringmsg.dismiss();
}

void read_int()
{
    int value = intmsg.data.get("value",0);
    std::cout << "client sent number: " << value << std::endl;
    intmsg.dismiss();
    endpoint.notify("ende");
}

void react(const EndpointEvent& ev)
{
    switch (ev.type)
    {
    case EventType::connected:
        std::cout << "Connected: " << endpoint.remote_ip().to_string() << std::endl;
        break;
    case EventType::message:
        if (ev.msg.name == "getName")
        {
            JSObject response;
            response.put("name","My Testserver");
            endpoint.respond(ev.msg.id,response);
            stringmsg = endpoint.request("getString");
            intmsg = endpoint.request("getNumber");
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
        if (stringmsg.update(endpoint.responses))
            read_string();
        if (intmsg.update(endpoint.responses))
            read_int();
    }
}