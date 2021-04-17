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

void SSL_Socket::set_client(const std::string& _sni)
{
    sni = _sni;
    clear();
    fill_client();
}

void SSL_Socket::set_server()
{
    role = SSL_Socket_Asio::handshake_type::server;
    clear();
    delete ssl_ctx;
    begin_server_context_creation();
    ssl_ctx = end_context_creation();
    fill_server();
}

void SSL_Socket::reset()
{
    clear();
    if (role==SSL_Socket_Asio::handshake_type::client)
        fill_client();
    else if (role==SSL_Socket_Asio::handshake_type::server)
        fill_server();
    else
        throw Exception("Ungueltige Rolle","SSL_Socket::reset()");
}

void SSL_Socket::clear()
{
    SSL_Socket_Asio* base = this;
    base->~SSL_Socket_Asio();
}

void SSL_Socket::fill_server()
{
    SSL_Socket_Asio* base = this;
    new (base) SSL_Socket_Asio(ioc,*ssl_ctx);
    set_verify_mode(boost::asio::ssl::verify_none);
}

void SSL_Socket::fill_client()
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