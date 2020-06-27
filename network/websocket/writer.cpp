#include "writer.h"

using namespace tobilib;
using namespace network;
using namespace detail;

WS_Writer::WS_Writer(
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket>& _socket,
    WS_writer_options* _options
    ): socket(_socket), options(_options)
{ }

void WS_Writer::tick()
{
    if (timeout.due())
    {
        timeout.disable();
        if (closing)
            events.push(WS_writer_event::close_timeout);
        else
            events.push(WS_writer_event::send_timeout);
        return;
    }
}

void WS_Writer::send_data(const std::string& msg)
{
    if (!is_good())
        return;
    data_queue += msg;
    flush();
}

void WS_Writer::send_close()
{
    if (!is_good())
        return;
    wannaclose = true;
    if (options.close_timeout>0)
        close_timeout.set(parent->options.close_timeout);
    flush();
}

bool WS_Writer::is_busy() const
{
    return writing;
}

void WS_Writer::reset()
{
    if (writing)
        throw Exception("Implementierungsfehler","network::detail::WS_Writer::reset()");
    data_queue.clear();
    data_sending.clear();
    wannaclose = false;
    closing = false;
    writing = false;
    timeout.disable();
    while (!events.empty())
        events.pop();
}

void WS_Writer::flush()
{
    if (is_busy())
        return;
    data_sending += data_queue;
    data_queue.clear();
    if (wannaclose)
    {
        parent->socket.async_close(
            boost::beast::websocket::close_reason("Server close-request"),
            boost::bind(&on_close_written,this,_1)
        );
        closing = true;
        writing = true;
    }
    else if (!data_sending.empty())
    {
        parent->socket.async_write(
            data_sending,
            boost::bind(&on_data_written,this,_1,_2)
        );
        writing = true;
    }
}

void WS_Writer::on_data_written(const boost::system::error_code& ec, size_t written)
{
    writing = false;
    if (ec)
    {
        events.push(WS_writer_event::send_error)
        return;
    }
    buffer_sending = buffer_sending.substr(written);
    flush();
}

void WS_Writer::on_close_written(const boost::system::error_code& ec)
{
    writing = false;
    closing = false;
    if (ec)
    {
        events.push(WS_writer_event::send_error)
        return;
    }
}