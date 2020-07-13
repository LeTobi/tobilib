#ifndef TC_WEBSOCKET_ENDPOINT_H
#define TC_WEBSOCKET_ENDPOINT_H

#include "../interface.h"
#include "./reader.h"
#include "./writer.h"

namespace tobilib {
namespace network {

class WS_Endpoint;

struct WS_Options: public detail::WS_writer_options, public detail::WS_reader_options
{
    double connection_timeout = 0;
};

class WS_Endpoint : public virtual Endpoint
{
public:
    WS_Endpoint();

    bool is_inactive() const;
    void write(const std::string&);
    std::string read(unsigned int len=0);
    std::string peek(unsigned int len=0) const;
    unsigned int recv_size() const;
    void close();

    WS_Options options;

protected:
    void ws_endpoint_tick();
    void ws_endpoint_abort();
    void ws_endpoint_reset();
    void start_reading();

    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket;

private:
    friend class detail::WS_Reader;
    friend class detail::WS_Writer;

    detail::WS_Reader reader;
    detail::WS_Writer writer;
};

} // namespace network
} // namespace tobilib

#endif