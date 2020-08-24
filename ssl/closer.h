#include "connector.h"
#include "../general/exception.hpp"
#include <boost/beast.hpp>

namespace boost {
namespace beast {
namespace websocket {
    template<class TeardownHandler>
    void async_teardown(
        boost::beast::role_type role,
        tobilib::network::detail::SSL_Stream& stream,
        TeardownHandler&& handler)
    {
        //async_teardown(role, (tobilib::network::detail::Asio_SSL_Stream&)stream, handler);
    }
} // namespace websocket
} // namespace beast
} // namespace boost

namespace tobilib {
namespace network {
namespace detail {

    class SSL_Closer
    {
    public:
        SSL_Closer(SSL_Stream&, boost::asio::io_context&,Logger&);

        void request();
        void force();
        void cleanup();

    private:
        SSL_Stream& socket;
        boost::asio::io_context& ioc;
        Logger& log;
        bool pending = false;

        void on_close(const boost::system::error_code&);
    };

    class WSS_Closer
    {
    public:
        using WSStream = boost::beast::websocket::stream<SSL_Stream>;

        WSS_Closer(WSStream&, boost::asio::io_context&, Logger&);

        void request();
        void force();
        void cleanup();

    private:
        WSStream& socket;
        boost::asio::io_context& ioc;
        Logger& log;
        bool pending = false;

        void on_close(const boost::system::error_code&);
    };

} // namespace detail
} // namespace network
} // namespace tobilib