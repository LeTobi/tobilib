#include "acceptor.h"
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include "../general/exception.hpp"

using namespace tobilib;
using namespace network;
using boost::placeholders::_1;

Acceptor::Acceptor(unsigned int port): acceptor(ioc), _port(port)
{
    boost::asio::ip::tcp::endpoint ep (boost::asio::ip::tcp::v4(), _port);
    acceptor.open(ep.protocol());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(ep);
    acceptor.listen();
}

bool Acceptor::busy() const
{
    return current != nullptr;
}

void Acceptor::connect(Interface* iface)
{
    if (iface!=nullptr)
        queue.push_back(iface);
    if (busy() || queue.empty())
        return;
    current = queue.front();
    queue.pop_front();
    acceptor.async_accept(current->socket,boost::bind(&Acceptor::on_connect,this,_1));
}

void Acceptor::on_connect(const boost::system::error_code& ec)
{
    current->finished = true;
    current->pending = false;
    current->error = ec;
    if (!ec)
        current->remote_ip = current->socket.remote_endpoint().address();
    current = nullptr;
    connect(nullptr);
}

Acceptor::Interface::Interface (
    Acceptor& _accpt,
    boost::asio::ip::tcp::socket& _socket,
    boost::asio::io_context& unused,
    ConnectorOptions& _options):
        accpt(_accpt),
        socket(_socket),
        Connector(_options)
{ }

void Acceptor::Interface::tick()
{
    if (accpt.current == this)
        accpt.ioc.poll_one();
}

void Acceptor::Interface::connect()
{
    finished = false;
    pending = true;
    accpt.connect(this);
}

void Acceptor::Interface::reset()
{
    socket.close();
    if (pending)
        accpt.queue.remove(this);
    finished = false;
    if (accpt.current!=this)
        return;
    accpt.acceptor.cancel();
    while (!finished)
        accpt.ioc.poll_one();
    finished = false;
    accpt.acceptor.listen();
}