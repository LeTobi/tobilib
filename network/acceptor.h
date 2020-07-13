#ifndef TC_NETWORK_ACCEPTOR
#define TC_NETWORK_ACCEPTOR

#include <boost/asio.hpp>

namespace tobilib {
namespace network {

    class Server_Endpoint;
    class WS_Server_Endpoint;

    class Acceptor {
    public:
        Acceptor(unsigned int);

    private:
        friend class Server_Endpoint;

        boost::asio::io_context ioc;
        boost::asio::ip::tcp::acceptor acceptor;
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ioc);
        unsigned int _port;
        bool occupied = false;
    };

} // namespace network
} // namespace tobilib

#endif