#include "network.h"

void tobilib::network::ssl_server_init(const std::string& pemfile)
{
    ssl_server_ctx.set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2);
    boost::system::error_code ec;
    ssl_server_ctx.use_certificate_chain_file(pemfile, ec);
    if (ec)
        throw Exception(std::string("Fehler beim Laden von Zertifikaten (1): ")+ec.message(),"ssl_server_init()");
    ssl_server_ctx.use_private_key_file(pemfile, boost::asio::ssl::context::pem, ec);
    if (ec)
        throw Exception(std::string("Fehler beim Laden von Zertifikaten (2): ")+ec.message(),"ssl_server_init()");
}

#define TC_SSL_IMPL_ONLY
#include "../network/network.cpp"
#include "../network/reader.cpp"
#include "../network/writer.cpp"
#include "../network/ws-connector.cpp"