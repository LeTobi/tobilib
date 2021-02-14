#include "../../protocols/h2rfp.h"
#include <iostream>

using namespace tobilib;
using namespace h2rfp;

TCP_Endpoint endpoint ("localhost",15432);
Response servername;

void read_string(const Message& req)
{
    std::cout << "Server is requesting a string: " << std::endl;
    std::string input;
    std::cin >> input;
    JSObject out;
    out.put("value",input);
    endpoint.respond(req.id,out);
}

void read_int(const Message& req)
{
    std::cout << "Server is requesting an integer: " << std::endl;
    int input;
    std::cin >> input;
    JSObject out;
    out.put("value",input);
    endpoint.respond(req.id,out);
}

void react(const EndpointEvent& ev)
{
    if (ev.type == EventType::connected)
    {
        servername = endpoint.request("getName");
        return;
    }

    if (ev.type != EventType::message)
        return;
    if (ev.msg.name == "getString")
        read_string(ev.msg);
    else if (ev.msg.name == "getNumber")
        read_int(ev.msg);
    else if (ev.msg.name == "ende")
        std::cout << "Server informiert: Interaktion ist zu ende." << std::endl;
    else
        std::cout << "unknown request: " << ev.msg.name << std::endl;
}

int main()
{
    std::cout << "H2RFP client supplementary to h2rfp-server" << std::endl;
    endpoint.connect();
    while (endpoint.status() != EndpointStatus::closed)
    {
        endpoint.tick();
        if (servername.update(endpoint.responses))
        {
            std::cout << "verbunden mit " << servername.data.get("name","") << std::endl;
            servername.dismiss();
        }
        while (!endpoint.events.empty())
            react(endpoint.events.next());
    }
    std::cout << "Program finished" << std::endl;
    return 0;
}