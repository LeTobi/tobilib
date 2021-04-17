#include "ssl-context.h"

#include "../general/exception.hpp"
#include <vector>
#include <mutex>

struct {

    std::string cert_file;
    std::vector<std::string> cert_paths;
    boost::asio::ssl::context* last_context = nullptr;
    std::mutex creation_lock;

} ssl_context_data;

void tobilib::network::set_cert_file(const std::string& file)
{
    if (!ssl_context_data.cert_file.empty())
        throw tobilib::Exception("Das pem-File sollte nur einmal gesetzt werden.","tobilib::network::set_cert_file()");
    ssl_context_data.cert_file = file;
}

void tobilib::network::add_cert_path(const std::string& path)
{
    ssl_context_data.cert_paths.push_back(path);
}

boost::asio::ssl::context* tobilib::network::detail::begin_server_context_creation()
{
    ssl_context_data.creation_lock.lock();
    if (ssl_context_data.last_context != nullptr)
        throw tobilib::Exception("Kontext Erstellungsprozess falsch angewendet","tobilib::network::begin_server_context_creation()");
    boost::asio::ssl::context* out = new boost::asio::ssl::context(boost::asio::ssl::context::sslv23);
    out->set_options(
        boost::asio::ssl::context::default_workarounds
        | boost::asio::ssl::context::no_sslv2);
    boost::system::error_code ec;
    out->use_certificate_chain_file(ssl_context_data.cert_file, ec);
    if (ec)
        throw Exception(std::string("Fehler beim Laden von Zertifikaten (1): ")+ec.message(),"tobilib::network::begin_server_context_creation()");
    out->use_private_key_file(ssl_context_data.cert_file, boost::asio::ssl::context::pem, ec);
    if (ec)
        throw Exception(std::string("Fehler beim Laden von Zertifikaten (2): ")+ec.message(),"tobilib::network::begin_server_context_creation()");
    ssl_context_data.last_context = out;
    return out;
}

boost::asio::ssl::context* tobilib::network::detail::begin_client_context_creation()
{
    ssl_context_data.creation_lock.lock();
    if (ssl_context_data.last_context != nullptr)
        throw tobilib::Exception("Kontext Erstellungsprozess falsch angewendet","tobilib::network::begin_server_context_creation()");
    boost::asio::ssl::context* out = new boost::asio::ssl::context(boost::asio::ssl::context::sslv23);
    ssl_context_data.last_context = out;
    return out;
}

boost::asio::ssl::context* tobilib::network::detail::end_context_creation()
{
    if (ssl_context_data.last_context == nullptr)
        throw tobilib::Exception("Es wurde kein Kontext begonnen.","tobilib::network::detail::end_context_creation()");
    boost::asio::ssl::context* out = ssl_context_data.last_context;
    ssl_context_data.last_context = nullptr;
    ssl_context_data.creation_lock.unlock();
    return out;
}