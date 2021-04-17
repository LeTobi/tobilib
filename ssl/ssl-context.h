#ifndef TC_SSL_CONTEXT_H
#define TC_SSL_CONTEXT_H

#include <string>
#include <boost/asio/ssl.hpp>

namespace tobilib{
namespace network{

    void set_cert_file(const std::string&);
    void add_cert_path(const std::string&);

namespace detail {

    boost::asio::ssl::context* begin_server_context_creation();
    boost::asio::ssl::context* begin_client_context_creation();
    boost::asio::ssl::context* end_context_creation();

} // namespace detail
} // namespace network
} // namespace tobilib

#endif