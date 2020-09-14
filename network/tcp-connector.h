#ifndef TC_TCP_H
#define TC_TCP_H

#include "../general/timer.hpp"
#include "alias.h"

namespace tobilib{
namespace network{

    struct ConnectorOptions
    {
        double handshake_timeout = 0;
    };

namespace detail
{
    template<class SocketType>
    class Connector
    {
    public:
        Connector(ConnectorOptions& _options): options(_options) {};

        boost::system::error_code error;
        boost::asio::ip::address remote_ip;
        bool finished = false;
        virtual void tick() = 0;
        virtual void connect() = 0;
        virtual bool is_async() const = 0;
        virtual void cancel() = 0;
        virtual void reset(SocketType*) = 0;
        virtual ~Connector() { }

    protected:
        ConnectorOptions& options;
    };

    class TCP_ClientConnector: public Connector<TCP_Socket>
    {
    public:
        TCP_ClientConnector(const std::string&, unsigned int, TCP_Socket*, boost::asio::io_context&, ConnectorOptions&);

        void tick();
        void connect();
        bool is_async() const;
        void cancel();
        void reset(TCP_Socket*);
        
    private:
        TCP_Socket* socket;
        boost::asio::io_context& ioc;
        boost::asio::ip::tcp::resolver resolver;
        std::string target_address;
        unsigned int target_port;
        bool async = false;
        bool resolving = false;
        bool resetting = false;

        void on_resolve(const boost::system::error_code&, boost::asio::ip::tcp::resolver::results_type);
        void on_connect(const boost::system::error_code&, const boost::asio::ip::tcp::endpoint&);
    };

    // TCP_ServerConnector see acceptor.h

} // namespace detail
} // namespace network
} // namespace tobilib

#endif