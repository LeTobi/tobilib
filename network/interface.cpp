#include "interface.h"
#include "acceptor.h"

#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

using namespace tobilib;
using namespace network;
using boost::placeholders::_1;
using boost::placeholders::_2;

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
    if (_status != Status::error)
        events.push(Event::failed);
    _status = Status::error;
    log << msg << std::endl;
}

void Endpoint::set_connecting()
{
    _status = Status::connecting;
}

void Endpoint::set_connected()
{
    if (_status != Status::connected)
        events.push(Event::connected);
    _status = Status::connected;
}

void Endpoint::set_closing()
{
    _status = Status::closing;
}

void Endpoint::set_closed()
{
    if (_status != Status::closed)
        events.push(Event::closed);
    _status = Status::closed;
}

void Endpoint::endpoint_abort()
{ }

void Endpoint::endpoint_reset()
{
    ioc.restart();
    events.clear();
    _status = Status::unused;
}

Server_Endpoint::Server_Endpoint(Acceptor& acceptor): _acceptor(acceptor)
{  
    _port = _acceptor._port;
}

Server_Endpoint::~Server_Endpoint()
{
    server_endpoint_abort();
}

void Server_Endpoint::server_endpoint_tick()
{
    if (true)
        _acceptor.ioc.poll_one();
}

void Server_Endpoint::tcp_connect(boost::asio::ip::tcp::socket& socket)
{
    if (_acceptor.occupied)
        throw Exception("Implementierungsfehler: Maximal 1 Endpoint in der Warteschlange","Server_Endpoint::tcp_connect()");
    tcp_connected = false;
    _acceptor.occupied = true;
    tcp_connecting = true;
    _acceptor.acceptor.async_accept(socket,boost::bind(&Server_Endpoint::on_connect,this,_1,&socket));
}

void Server_Endpoint::server_endpoint_abort()
{
    if (tcp_connecting)
    {
        _acceptor.acceptor.cancel();
        while (tcp_connecting)
            _acceptor.ioc.poll_one();
        _acceptor.acceptor.listen();
    }
}

void Server_Endpoint::server_endpoint_reset()
{ 
    tcp_connected = false;
}

void Server_Endpoint::on_connect(const boost::system::error_code& ec, boost::asio::ip::tcp::socket* socket)
{
    _acceptor.occupied=false;
    tcp_connecting = false;
    tcp_connected = true;
    tcp_result = ec;
    if (ec)
        return;
    try {
        _remote_ip = socket->remote_endpoint().address();
    } catch (boost::system::system_error& err) {
        log << err.what() << std::endl;
    }
}

Client_Endpoint::Client_Endpoint(const std::string& host, unsigned int port): resolver(ioc)
{
    _host = host;
    _port = port;
}

void Client_Endpoint::tcp_connect(boost::asio::ip::tcp::socket& socket)
{
    tcp_connecting = true;
    socket_to_connect = &socket;
    resolver.async_resolve(_host,std::to_string(_port),boost::bind(&Client_Endpoint::on_resolve,this,_1,_2));
}

void Client_Endpoint::client_endpoint_tick()
{ }

void Client_Endpoint::client_endpoint_abort()
{
    if (tcp_connecting)
    {
        resolver.cancel();
        socket_to_connect->close();
        while (tcp_connecting)
            ioc.poll_one();
    }
}

void Client_Endpoint::client_endpoint_reset()
{
    tcp_connected = false;
}

void Client_Endpoint::on_resolve(
    const boost::system::error_code& ec,
    boost::asio::ip::tcp::resolver::results_type results)
{
    if (ec)
    {
        tcp_result = ec;
        tcp_connecting = false;
        tcp_connected = true;
        return;
    }
    boost::asio::async_connect(*socket_to_connect,results,boost::bind(&Client_Endpoint::on_connect,this,_1,_2));
}

void Client_Endpoint::on_connect(const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& endpoint)
{
    _remote_ip = endpoint.address();
    tcp_result = ec;
    tcp_connecting = false;
    tcp_connected = true;
}