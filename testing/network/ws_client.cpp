#include <iostream>
#include "../../network/websocket/clientendpoint.h"

using namespace tobilib;
using namespace network;

void send(WS_Client_Endpoint* ep)
{
    std::cout << "your message: ";
    std::string msg;
    std::cin >> msg;
    ep->write(msg);
}

int main()
{
    std::cout << "***Synchronous Websocket-Client***" << std::endl;
    std::cout << "enter address:" << std::endl;
    std::string host;
    std::cin >> host;
    std::cout << "enter port:" << std::endl;
    unsigned int port;
    std::cin >> port;

    WS_Client_Endpoint endpoint (host,port);
    endpoint.options.connection_timeout = 5;
    endpoint.connect();
    while (true)
    {
        endpoint.tick();
        while (!endpoint.events.empty())
            switch (endpoint.events.next())
            {
            case Event::connected:
                std::cout << "Connected to " << endpoint.remote_ip().to_string() << std::endl;
                send(&endpoint);
                break;
            case Event::read:
                std::cout << "Server: " << endpoint.read() << std::endl;
                send(&endpoint);
                break;
            case Event::inactive:
                // nothing
                break;
            case Event::closed:
                std::cout << "Connection closed" << std::endl;
                return 0;
            case Event::failed:
                return 0;
            }
    }
}