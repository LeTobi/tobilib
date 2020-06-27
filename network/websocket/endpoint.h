#ifndef TC_WEBSOCKET_ENDPOINT_H
#define TC_WEBSOCKET_ENDPOINT_H

#include "../interface.h"
#include "./reader.h"
#include "./writer.h"

namespace tobilib {
namespace network {

class WS_Endpoint;

struct WS_Options: public details::WS_writer_options, public details::WS_reader_options
{
    double connection_timeout = 0;
}

class WS_Endpoint : public virtual Endpoint
{
public:
    WS_Endpoint();

    bool is_inactive() const;
    virtual void tick();
    void write(const std::string&);
    std::string read(unsigned int len=0);
    std::string peek(unsigned int len=0) const;
    unsigned int recv_size() const;
    void close();
    void abort();
    void reset();

    WS_Options options;

protected:
    void ws_endpoint_tick();
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