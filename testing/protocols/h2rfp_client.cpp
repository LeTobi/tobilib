#include "../../protocols/h2rfp.h"
#include <iostream>

using namespace tobilib;
using namespace h2rfp;

TCP_Endpoint endpoint ("localhost",15432);

void read_string(const Message& req)
{
    std::cout << "Server is requesting a string: " << std::endl;
    std::string input;
    std::cin >> input;
    Message out;
    out.id = req.id;
    out.data.put("value",input);
    endpoint.send(out);
}

void read_int(const Message& req)
{
    std::cout << "Server is requesting an integer: " << std::endl;
    int input;
    std::cin >> input;
    Message out;
    out.id = req.id;
    out.data.put("value",input);
    endpoint.send(out);
}

void react(const EndpointEvent& ev)
{
    if (ev.type != EventType::request)
        return;
    if (ev.msg.name == "getString")
        read_string(ev.msg);
    else if (ev.msg.name == "getNumber")
        read_int(ev.msg);
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
        while (!endpoint.events.empty())
            react(endpoint.events.next());
    }
    std::cout << "Program finished" << std::endl;
    return 0;
}