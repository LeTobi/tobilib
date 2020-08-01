#include "tcp.h"
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

using namespace tobilib;
using namespace network;
using namespace detail;
using boost::placeholders::_1;
using boost::placeholders::_2;


TCP_Client_Connect::TCP_Client_Connect(
    const std::string& _address,
    unsigned int _port,
    boost::asio::ip::tcp::socket& _socket,
    boost::asio::io_context& _ioc,
    ConnectorOptions& _options
    ):
        socket(_socket),
        ioc(_ioc),
        resolver(_ioc),
        target_address(_address),
        target_port(_port),
        Connector(_options)
{ }

void TCP_Client_Connect::tick()
{ }

void TCP_Client_Connect::connect()
{
    finished = false;
    error.clear();
    active = true;
    resolver.async_resolve(
        target_address,
        std::to_string(target_port),
        boost::bind(&TCP_Client_Connect::on_resolve,this,_1,_2)
        );
}

void TCP_Client_Connect::reset()
{
    resolver.cancel();
    socket.close();
    while(active)
        ioc.poll_one();
    error.clear();
    finished = false;
}

void TCP_Client_Connect::on_resolve(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results)
{
    error = ec;
    if (error)
    {
        finished = true;
        active = false;
        return;
    }
    boost::asio::async_connect(socket,results,boost::bind(&TCP_Client_Connect::on_connect,this,_1,_2));
}

void TCP_Client_Connect::on_connect(const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& ep)
{
    error = ec;
    remote_ip = ep.address();
    finished = true;
    active = false;
}

Connector::~Connector()
{ }