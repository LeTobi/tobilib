#include <iostream>
#include "../../network/interface.h"

using namespace tobilib;
using namespace network;

void send(TCP_Endpoint* ep)
{
    std::cout << "your message: ";
    std::string msg;
    std::cin >> msg;
    msg += "\n";
    ep->write(msg);
}

int main()
{
    std::cout << "***Synchronous TCP-Client***" << std::endl;
    std::cout << "enter address:" << std::endl;
    std::string host;
    std::cin >> host;
    std::cout << "enter port:" << std::endl;
    unsigned int port;
    std::cin >> port;

    TCP_Endpoint endpoint (host,port);
    endpoint.options.connect_timeout = 5;
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