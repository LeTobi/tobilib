#include <iostream>
#include "../../ssl/network.h"

using namespace tobilib;
using namespace network;
using namespace detail;

int main()
{
    set_cert_file("/root/ssl/wetterfrosch.pem");
    Acceptor accpt (1533);
    WSS_Endpoint endpoint (accpt);
    endpoint.connect();
    std::cout << "WSS echoserver auf Port 1533" << std::endl;
    Timer cutofftimer (10);
    while (true)
    {
        endpoint.tick();
        while (!endpoint.events.empty())
        switch (endpoint.events.next())
        {
        case EndpointEvent::connected:
            std::cout << "Verbunden mit " << endpoint.remote_ip().to_string() << std::endl;
            cutofftimer.set();
            break;
        case EndpointEvent::received:
            std::cout << "Nachricht erhalten: " << endpoint.peek() << std::endl;
            endpoint.write(endpoint.read());
            break;
        case EndpointEvent::inactive:
            break;
        case EndpointEvent::closed:
            std::cout << "Verbindung getrennt" << std::endl;
            endpoint.connect();
            break;
        }

        if (cutofftimer.due())
        {
            std::cout << "connection abort" << std::endl;
            cutofftimer.disable();
            endpoint.reset();
        }
    }
}