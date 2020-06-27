#include "interface.h"
#include "acceptor.h"

#include <boost/bind.hpp>

using namespace tobilib;
using namespace network;

Endpoint::Status Endpoint::status() const
{
    return _status;
}

bool Endpoint::is_good() const
{
    return _status != Status::error;
}

bool Endpoint::is_connected() const
{
    return _status == Status::connected;
}

bool Endpoint::is_closed() const
{
    return _status == Status::closed;
}

boost::asio::ip::address Endpoint::remote_ip() const
{
    return _remote_ip;
}

void Endpoint::fail(const std::string& msg)
{
    _status = Status::error;
    log << msg << std::endl;
    events.push(Event::failed);
}

void Endpoint::set_unused()
{
    _status = Status::unused;
}

void Endpoint::set_connecting()
{
    _status = Status::connecting;
}

void Endpoint::set_connected()
{
    _status = Status::connected;
    events.push(Event::connected);
}

void Endpoint::set_closing()
{
    _status = Status::closing;
}

void Endpoint::set_closed()
{
    _status = Status::closed;
    events.push(Event::closed);
}

Server_Endpoint::Server_Endpoint(Acceptor& acceptor): _acceptor(acceptor)
{  
    _port = _acceptor._port;
}

void Server_Endpoint::server_endpoint_tick()
{
    _acceptor.ioc.poll_one();
}

void Server_Endpoint::tcp_connect(boost::asio::ip::tcp::socket& socket)
{
    if (_acceptor.occupied)
        throw Exception("Implementierungsfehler: Maximal 1 Endpoint in der Warteschlange","Server_Endpoint::tcp_connect()");
    tcp_connected = false;
    _acceptor.occupied = true;
    _acceptor.acceptor.async_accept(socket,boost::bind(&Server_Endpoint::on_connect,this,_1,socket));
}

void Server_Endpoint::on_connect(const boost::system::error_code& ec, boost::asio::ip::tcp::socket& socket)
{
    _acceptor.occupied=false;
    tcp_connected = true;
    tcp_result = ec;
    if (ec)
        return;
    try {
        _remote_ip = socket.remote_endpoint().address();
    } catch (boost::system::system_error& err) {
        log << err.what() << std::endl;
    }
}

Client_Endpoint::Client_Endpoint(const std::string& ip, unsigned int port)
{
    _remote_ip.from_string(ip);
    _port = port;
}