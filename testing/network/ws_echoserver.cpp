#include <iostream>
#include "../../network/websocket/serverendpoint.h"

using namespace tobilib;
using namespace network;

int main()
{
    std::cout << "Echo server auf Port 15432" << std::endl;
    Acceptor acceptor(15432);
    WS_Server_Endpoint endpoint(acceptor);
    endpoint.options.inactive_shutdown=5;
    endpoint.connect();
    while (true)
    {
        endpoint.tick();
        if (!endpoint.events.empty())
            std::cout << "----" << std::endl;
        while (!endpoint.events.empty())
        {
            switch(endpoint.events.next())
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
                std::cout << "Verbindung getrennt... restart" << std::endl;
                endpoint.reset();
                endpoint.connect();
                break;
            case Event::failed:
                std::cout << "restart" << std::endl;
                endpoint.reset();
                endpoint.connect();
                break;
            }
        }
    }
}