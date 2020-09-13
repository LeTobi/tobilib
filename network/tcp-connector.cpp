#include "tcp-connector.h"
#include <boost/bind/bind.hpp>
#include "../general/exception.hpp"

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;


TCP_ClientConnector::TCP_ClientConnector(
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

void TCP_ClientConnector::tick()
{ }

void TCP_ClientConnector::connect()
{
    finished = false;
    error.clear();
    resolving = true;
    resolver.async_resolve(
        target_address,
        std::to_string(target_port),
        boost::bind(&TCP_ClientConnector::on_resolve,this,_1,_2)
        );
}

bool TCP_ClientConnector::is_async() const
{
    return async;
}

void TCP_ClientConnector::reset()
{
    if (async)
        throw Exception("reset mit ausstehender Verbindung","TCP_ClientConnector::reset()");
    resetting = true;
    if (resolving)
        resolver.cancel();
    while (resolving)
        ioc.poll_one();
    resetting = false;
    error.clear();
    finished = false;
}

void TCP_ClientConnector::on_resolve(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results)
{
    resolving = false;
    error = ec;
    if (error || resetting)
    {
        finished = true;
        return;
    }
    async = true;
    boost::asio::async_connect(socket,results,boost::bind(&TCP_ClientConnector::on_connect,this,_1,_2));
}

void TCP_ClientConnector::on_connect(const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& ep)
{
    error = ec;
    remote_ip = ep.address();
    finished = true;
    async = false;
}

Connector::~Connector()
{ }