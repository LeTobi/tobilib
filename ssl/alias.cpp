#include "alias.h"
#include "../general/exception.hpp"

boost::asio::ssl::context tobilib::network::ssl_client_ctx (boost::asio::ssl::context::sslv23);
boost::asio::ssl::context tobilib::network::ssl_server_ctx (boost::asio::ssl::context::sslv23);

using namespace tobilib;
using namespace network;
using namespace detail;

SSL_Socket::SSL_Socket(boost::asio::io_context& _ioc): SSL_Socket_Asio(_ioc,ssl_client_ctx), ioc(_ioc)
{ }

void SSL_Socket::reassign(int _role,const std::string& _sni)
{
    SSL_Socket_Asio* base = this;
    role = _role;
    sni = _sni;
    this->~SSL_Socket_Asio();
    if (role==SSL_Socket_Asio::handshake_type::client)
    {
        new (base) SSL_Socket_Asio(ioc,ssl_client_ctx);

        // openssl muss dem server die gewünschte Domain mitteilen
        if (!SSL_set_tlsext_host_name(native_handle(), sni.c_str()))
            throw Exception("SNI konnte nicht gesetzt werden","SSL_Socket::reassign()");
            
        set_verify_mode(boost::asio::ssl::verify_peer);
        //set_verify_mode(boost::asio::ssl::verify_none);
        set_verify_callback(boost::asio::ssl::host_name_verification(sni));
    }
    else if (role==SSL_Socket_Asio::handshake_type::server)
    {
        new (base) SSL_Socket_Asio(ioc,ssl_server_ctx);
        set_verify_mode(boost::asio::ssl::verify_none);
    }
    else
    {
        throw Exception("Ungueltige Rolle","SSL_Socket::reassign()");
    }
}