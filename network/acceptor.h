#ifndef TC_NETWORK_ACCEPTOR
#define TC_NETWORK_ACCEPTOR

#include <list>
#include "alias.h"
#include "tcp-connector.h"


namespace tobilib {
namespace network {

    class Acceptor {
    public:

        class Interface: public detail::Connector<detail::TCP_Socket>
        {
        public:
            Interface(Acceptor&, detail::TCP_Socket*, boost::asio::io_context&, ConnectorOptions&);

            void tick();
            void connect();
            void cancel();
            void reset(detail::TCP_Socket*);
        
        private:
            friend class Acceptor;

            Acceptor& accpt;
            detail::TCP_Socket* socket;
            bool enqueued = false;
        };

        Acceptor(unsigned int);
        bool busy() const;

    private:
        friend class Interface;

        Interface* current = nullptr;
        std::list<Interface*> queue;
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ioc);
        unsigned int _port;

        void connect(Interface*);
        void on_connect(const boost::system::error_code&);
    };

    using TCP_ServerConnector = Acceptor::Interface;

} // namespace network
} // namespace tobilib

#endif