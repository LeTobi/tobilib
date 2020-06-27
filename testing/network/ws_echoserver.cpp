#include "../../network/websocket/serverendpoint.h"
#include <iostream>

using namespace tobilib;
using namespace network;

int main()
{
    std::cout << "Echo server auf Port 15432" << std::endl;
    Acceptor acceptor(15432);
    WS_Server_Endpoint endpoint(acceptor);
    endpoint.connect();
    while (true)
    {
        endpoint.tick();
        if (endpoint.events.empty())
            continue;
        switch(endpoint.events.front())
        {
        case Event::connected:
            std::cout << "Verbunden mit " << endpoint.remote_ip().to_string() << std::endl;
            break;
        case Event::read:
            std::cout << "Nachricht erhalten: " << endpoint.peek(endpoint.recv_size()) << std::endl;
            endpoint.write(endpoint.read());
            break;
        case Event::inactive:
            std::cout << "Achtung, inaktiver Punkt" << std::endl;
            break;
        case Event::closed:
            std::cout << "Verbindung getrennt" << std::endl;
            endpoint.reset();
            endpoint.connect();
            break;
        case Event::failed:
            endpoint.reset();
            endpoint.connect();
            break;
        }
        endpoint.events.pop();
    }
}