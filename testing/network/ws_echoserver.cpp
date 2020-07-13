#include <iostream>
#include "../../network/websocket/serverendpoint.h"

using namespace tobilib;
using namespace network;

void reconnect(WS_Server_Endpoint* ep, Acceptor& accpt)
{
    ep->reset(); /*
    delete ep;
    ep = new WS_Server_Endpoint(accpt);
    //*/
    ep->connect();
}

int main()
{
    std::cout << "Echo server auf Port 15432" << std::endl;
    Acceptor acceptor(15432);
    WS_Server_Endpoint* endpoint = new WS_Server_Endpoint(acceptor);
    endpoint->options.inactive_shutdown=10;
    endpoint->options.connection_timeout=5;
    endpoint->connect();
    Timer dummytimer (30);
    dummytimer.set();
    while (true)
    {
        if (dummytimer.due())
        {
            dummytimer.disable();
            std::cout << "Servertimeout" << std::endl;
            endpoint->abort();
            return 0;
        }
        endpoint->tick();
        while (!endpoint->events.empty())
        {
            switch(endpoint->events.next())
            {
            case Event::connected:
                dummytimer.disable();
                std::cout << "Verbunden mit " << endpoint->remote_ip().to_string() << std::endl;
                break;
            case Event::read:
                std::cout << "Nachricht erhalten: " << endpoint->peek() << std::endl;
                endpoint->write(endpoint->read());
                break;
            case Event::inactive:
                std::cout << "Achtung, inaktiver Punkt" << std::endl;
                break;
            case Event::closed:
                std::cout << "Verbindung getrennt... restart" << std::endl;
                reconnect(endpoint,acceptor);
                dummytimer.set();
                break;
            case Event::failed:
                std::cout << "restart" << std::endl;
                reconnect(endpoint,acceptor);
                dummytimer.set();
                break;
            }
        }
    }
}