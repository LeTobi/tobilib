#include "acceptor.h"

using namespace tobilib;
using namespace network;

Acceptor::Acceptor(unsigned int port): acceptor(ioc), _port(port)
{
    boost::asio::ip::tcp::endpoint ep (boost::asio::ip::tcp::v4(), _port);
    acceptor.open(ep.protocol());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(ep);
    acceptor.listen();
}