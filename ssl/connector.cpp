#include "connector.h"
#include <boost/bind/bind.hpp>
#include "../network/errors.h"
#include "../general/exception.hpp"
#include <iostream>

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

boost::asio::ssl::context tobilib::network::ssl_client_ctx (boost::asio::ssl::context::sslv23);
boost::asio::ssl::context tobilib::network::ssl_server_ctx (boost::asio::ssl::context::sslv23);

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

SSL_Stream::SSL_Stream(boost::asio::io_context& _ioc): Asio_SSL_Stream(_ioc,ssl_client_ctx), ioc(_ioc)
{ }

void SSL_Stream::reassign(int _role,const std::string& _sni)
{
    Asio_SSL_Stream* base = this;
    this->~Asio_SSL_Stream();
    role = _role;
    sni = _sni;
    if (role==Asio_SSL_Stream::handshake_type::client)
    {
        new (base) Asio_SSL_Stream(ioc,ssl_client_ctx);
        if (!SSL_set_tlsext_host_name(native_handle(), sni.c_str()))
            throw Exception("SNI konnte nicht gesetzt werden","SSL_Stream::reassign()");
        set_verify_mode(boost::asio::ssl::verify_peer);
        //set_verify_mode(boost::asio::ssl::verify_none);
        set_verify_callback(boost::asio::ssl::host_name_verification(sni));
    }
    else if (role==Asio_SSL_Stream::handshake_type::server)
    {
        new (base) Asio_SSL_Stream(ioc,ssl_server_ctx);
        set_verify_mode(boost::asio::ssl::verify_none);
    }
    else
    {
        throw Exception("Ungueltige Rolle","SSL_Stream::reassign()");
    }
}

SSL_Client_Connect::SSL_Client_Connect(
    const std::string& address,
    unsigned int port,
    SSL_Stream& _socket,
    boost::asio::io_context& ioc,
    ConnectorOptions& _options):
        Connector(_options),
        socket(_socket),
        lowerlevel(address,port,_socket.next_layer(),ioc,_options)
{
    socket.reassign(Asio_SSL_Stream::handshake_type::client,address);
}

SSL_Server_Connect::SSL_Server_Connect(
    Acceptor& accpt,
    SSL_Stream& _socket,
    boost::asio::io_context& ioc,
    ConnectorOptions& _options):
        Connector(_options),
        socket(_socket),
        lowerlevel(accpt,_socket.next_layer(),ioc,_options)
{
    socket.reassign(Asio_SSL_Stream::handshake_type::server,"");
}

void SSL_Client_Connect::tick()
{
    lowerlevel.tick();
    if (lowerlevel.finished)
    {
        lowerlevel.finished = false;
        if (lowerlevel.error)
        {
            error = lowerlevel.error;
            finished = true;
            return;
        }
        if (options.handshake_timeout>0)
            hs_timer.set(options.handshake_timeout);
        remote_ip = lowerlevel.remote_ip;
        async = true;
        socket.async_handshake(
                Asio_SSL_Stream::handshake_type::client,
                boost::bind(&SSL_Client_Connect::on_handshake,this,_1)
            );
    }
    if (hs_timer.due())
    {
        hs_timer.disable();
        finished = true;
        error = boost::system::error_code(TobilibErrors.handshake_timeout,TobilibErrors);
    }
}

void SSL_Server_Connect::tick()
{
    lowerlevel.tick();
    if (lowerlevel.finished)
    {
        lowerlevel.finished = false;
        if (lowerlevel.error)
        {
            error = lowerlevel.error;
            finished = true;
            return;
        }
        if (options.handshake_timeout>0)
            hs_timer.set(options.handshake_timeout);
        remote_ip = lowerlevel.remote_ip;
        async = true;
        socket.async_handshake(
                Asio_SSL_Stream::handshake_type::server,
                boost::bind(&SSL_Server_Connect::on_handshake,this,_1)
            );
    }
    if (hs_timer.due())
    {
        hs_timer.disable();
        finished = true;
        error = boost::system::error_code(TobilibErrors.handshake_timeout,TobilibErrors);
    }
}

void SSL_Client_Connect::connect()
{
    finished = false;
    error.clear();
    lowerlevel.connect();
}

void SSL_Server_Connect::connect()
{
    finished = false;
    error.clear();
    lowerlevel.connect();
}

bool SSL_Client_Connect::is_async() const
{
    return async || lowerlevel.is_async();
}

bool SSL_Server_Connect::is_async() const
{
    return async || lowerlevel.is_async();
}

void SSL_Client_Connect::reset()
{
    if (is_async())
        throw Exception("reset mit ausstehender Operation","SSL_Client_Connect::reset()");
    hs_timer.disable();
    lowerlevel.reset();
    finished = false;
}

void SSL_Server_Connect::reset()
{
    if (is_async())
        throw Exception("reset mit ausstehender Operation","SSL_Server_Connect::reset()");
    hs_timer.disable();
    lowerlevel.reset();
    finished = false;
}

void SSL_Client_Connect::on_handshake(const boost::system::error_code& ec)
{
    hs_timer.disable();
    async = false;
    finished = true;
    error = ec;
}

void SSL_Server_Connect::on_handshake(const boost::system::error_code& ec)
{
    hs_timer.disable();
    async = false;
    finished = true;
    error = ec;
}