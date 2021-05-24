#include "alias.h"
#include "../general/exception.hpp"
#include "ssl-context.h"

using namespace tobilib;
using namespace network;
using namespace detail;

SSL_Socket::SSL_Socket(boost::asio::io_context& _ioc):
    SSL_Socket_Asio(_ioc,*begin_client_context_creation()),
    ssl_ctx(end_context_creation()),
    role(SSL_Socket_Asio::handshake_type::client),
    ioc(_ioc)
{ }

SSL_Socket::~SSL_Socket()
{
    delete ssl_ctx;
}

void SSL_Socket::setup_client(const std::string& _sni)
{
    if (role==SSL_Socket_Asio::handshake_type::server)
        throw Exception("Die Rueckkonvertierung von server zu client ist nicht implementiert","SSL_Socket::setup_client()");
    sni = _sni;
    delete_base();
    new_base_client();
}

void SSL_Socket::setup_server()
{
    delete_base();
    if (role != SSL_Socket_Asio::handshake_type::server) {
        role = SSL_Socket_Asio::handshake_type::server;
        delete ssl_ctx;
        begin_server_context_creation();
        ssl_ctx = end_context_creation();
    }
    new_base_server();
}

void SSL_Socket::reset()
{
    if (role == SSL_Socket_Asio::handshake_type::client)
        setup_client(sni);
    else if (role == SSL_Socket_Asio::handshake_type::server)
        setup_server();
    else
        throw Exception("Unbekannte Rolle","SSL_Socket::reset()");
}

void SSL_Socket::delete_base()
{
    SSL_Socket_Asio* base = this;
    base->~SSL_Socket_Asio();
}

void SSL_Socket::new_base_server()
{
    SSL_Socket_Asio* base = this;
    new (base) SSL_Socket_Asio(ioc,*ssl_ctx);
    set_verify_mode(boost::asio::ssl::verify_none);
}

void SSL_Socket::new_base_client()
{
    SSL_Socket_Asio* base = this;
    new (base) SSL_Socket_Asio(ioc,*ssl_ctx);

    // openssl muss dem server die gewünschte Domain mitteilen
    if (!SSL_set_tlsext_host_name(native_handle(), sni.c_str()))
        throw Exception("SNI konnte nicht gesetzt werden","SSL_Socket::reset()");
        
    set_verify_mode(boost::asio::ssl::verify_peer);
    //set_verify_mode(boost::asio::ssl::verify_none);
    set_verify_callback(boost::asio::ssl::host_name_verification(sni));
}