#ifndef TC_NETWORK_ENDPOINT_H
#define TC_NETWORK_ENDPOINT_H

#include <string>
#include <sstream>
#include <boost/asio.hpp>
#include <queue>
#include "../general/exception.hpp"
#include "acceptor.h"

namespace tobilib {
namespace network {

enum class Event
{
    connected,
    read,
    inactive,
    closed,
    failed
};

class Endpoint
{
public:
    enum class Status {
        unused,
        connecting,
        connected,
        closing,
        closed,
        error
    };

    Logger log = std::string("network-Endpoint: ");
    Status status() const;
    bool is_good() const;
    bool is_connected() const;
    bool is_closed() const;
    boost::asio::ip::address remote_ip() const;

    virtual void tick() = 0;
    virtual void connect() = 0;
    virtual void write(const std::string&) = 0;
    virtual std::string read(unsigned int) = 0;
    virtual std::string peek(unsigned int) const = 0;
    virtual unsigned int recv_size() const = 0;
    virtual void close() = 0;
    virtual void abort() = 0;
    virtual void reset() = 0;
    
    std::queue<Event> events;

protected:
    void fail(const std::string&);
    void set_unused();
    void set_connecting();
    void set_connected();
    void set_closing();
    void set_closed();

    boost::asio::io_context ioc;
    boost::asio::ip::address _remote_ip;
    unsigned int _port;

private:
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ioc);
    Status _status = Status::closed;

};

class Server_Endpoint : public virtual Endpoint
{
public:
    Server_Endpoint(Acceptor&);

protected:
    Acceptor& _acceptor;
    void server_endpoint_tick();
    void tcp_connect(boost::asio::ip::tcp::socket&);
    bool tcp_connected = false;
    boost::system::error_code tcp_result;

private:
    void on_connect(const boost::system::error_code&, boost::asio::ip::tcp::socket&);
};

class Client_Endpoint : public virtual Endpoint
{
public:
    Client_Endpoint(const std::string&, unsigned int);

};

} // namespace network
} // namespace tobilib

#endif