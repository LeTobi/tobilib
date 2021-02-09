#include <iostream>
#include "../../ssl/network.h"

using namespace tobilib;
using namespace network;

void send(WSS_Endpoint* ep)
{
    std::cout << "your message: ";
    std::string msg;
    std::cin >> msg;
    ep->write(msg);
}

int main()
{
    ssl_client_ctx.set_default_verify_paths();

    std::cout << "***Synchronous WSS-Client***" << std::endl;
    std::cout << "enter address:" << std::endl;
    std::string host;
    std::cin >> host;
    std::cout << "enter port:" << std::endl;
    unsigned int port;
    std::cin >> port;

    WSS_Endpoint endpoint (host,port);
    endpoint.options.connect_timeout = 30;
    endpoint.connect();
    while (true)
    {
        endpoint.tick();
        while (!endpoint.events.empty())
        switch (endpoint.events.next())
        {
        case EndpointEvent::connected:
            std::cout << "Connected to " << endpoint.remote_ip().to_string() << std::endl;
            send(&endpoint);
            break;
        case EndpointEvent::received:
            std::cout << "Server: " << endpoint.read() << std::endl;
            send(&endpoint);
            break;
        case EndpointEvent::inactive:
            // nothing
            break;
        case EndpointEvent::closed:
            std::cout << "Connection closed" << std::endl;
            return 0;
        }
    }
}