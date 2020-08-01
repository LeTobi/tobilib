#include <iostream>
#include "../../network/interface.h"

using namespace tobilib;
using namespace network;

int main()
{
    Acceptor accpt (15432);
    TCP_Endpoint endpoint (accpt);
    endpoint.options.inactive_warning = 5;
    endpoint.options.read_timeout = 10;
    endpoint.connect();
    std::cout << "TCP echoserver auf Port 15432" << std::endl;
    while (true)
    {
        endpoint.tick();
        while (!endpoint.events.empty())
        switch (endpoint.events.next())
        {
        case EndpointEvent::connected:
            std::cout << "Verbunden mit " << endpoint.remote_ip().to_string() << std::endl;
            break;
        case EndpointEvent::received:
            std::cout << "Nachricht erhalten: " << endpoint.peek() << std::endl;
            endpoint.write(endpoint.read());
            break;
        case EndpointEvent::inactive:
            std::cout << "Verbindung inaktiv" << std::endl;
            break;
        case EndpointEvent::closed:
            std::cout << "Verbindung getrennt" << std::endl;
            endpoint.connect();
            break;
        }
    }
}