#ifndef TC_NETWORK_CLOSER_H
#define TC_NETWORK_CLOSER_H

#include "../general/exception.hpp"
#include "alias.h"

namespace tobilib{
namespace network{
namespace detail {
    class TCP_Closer {
    public:
        TCP_Closer(TCP_Socket*,boost::asio::io_context&);

        void request();
        void force();
        bool is_closing() const;
        void reset(TCP_Socket*);

        boost::system::error_code error;

    private:
        boost::asio::io_context& ioc;
        TCP_Socket* socket;
    };

    class WS_Closer
    {
    public:
        WS_Closer(WS_Socket*, boost::asio::io_context&);

        void request();
        void force();
        bool is_closing() const;
        void reset(WS_Socket*);

        boost::system::error_code error;

    private:
        WS_Socket* socket;
        boost::asio::io_context& ioc;
        bool asio_closing = false;

        void on_close(const boost::system::error_code&);
    };

} // namespace detail
} // namespace network
} // namespace tobilib

#endif