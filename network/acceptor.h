#ifndef TC_NETWORK_ACCEPTOR
#define TC_NETWORK_ACCEPTOR

#include <boost/asio.hpp>
#include "tcp.h"
#include <list>

namespace tobilib {
namespace network {

    class Acceptor {
    public:

        class Interface: public detail::Connector
        {
        public:
            Interface(Acceptor&, boost::asio::ip::tcp::socket&, boost::asio::io_context&, ConnectorOptions&);

            void tick();
            void connect();
            bool is_async() const;
            void reset();
        
        private:
            friend class Acceptor;

            Acceptor& accpt;
            boost::asio::ip::tcp::socket& socket;
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

} // namespace network
} // namespace tobilib

#endif