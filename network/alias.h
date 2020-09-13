#ifndef TC_NETWORK_ALIAS_H
#define TC_NETWORK_ALIAS_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace tobilib {
namespace network {
namespace detail {

    using TCP_Socket = boost::asio::ip::tcp::socket;
    using WS_Socket = boost::beast::websocket::stream<TCP_Socket>;

} // namespace detail
} // namespace network
} // namespace tobilib

#endif