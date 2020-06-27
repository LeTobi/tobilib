#include "reader.h"

using namespace tobilib;
using namespace network;
using namespace detail;

WS_Reader::WS_Reader(WS_Endpoint* wse): parent(wse)
{ }

void WS_Reader::tick()
{
    if (inactive_timer.due())
    {
        inactive_timer.disable();
        inactive = true;
        events.push(WS_reader_event::inactive_warning)
    }
    if (deadline_timer.due())
    {
        deadline_timer.disable();
        events.push(WS_reader_event::inactive_shutdown)
        return;
    }
}

bool WS_Reader::is_inactive() const
{
    return inactive;
}

void WS_Reader::start_reading()
{
    if (reading)
        return;
    if (options.inactive_warning>0)
        inactive_timer.set(options.inactive_timeout);
    if (options.inactive_shutdown>0)
        deadline_timer.set(options.inactive_shutdown);
    parent->socket.async_read(
        buffer,
        boost::bind(&on_data_read,this,_1)
    );
}

void WS_Reader::reset()
{
    if (reading)
        throw Exception("implementierungsfehler","network::detail::WS_Reader::reset()");
    data.clear();
    buffer.consume(buffer.size());
    inactive_timer.disable();
    deadline_timer.disable();
    inactive=false;
    while (!events.empty())
        events.pop();
}

void WS_Reader::on_data_read(const boost::system::error_code& ec, size_t read)
{
    reading = false;
    inactive = false;
    inactive_timer.disable();
    deadline_timer.disable();
    if (ec)
    {
        if (ec.value() == (int)boost::beast::websocket::error::closed) {
            events.push(WS_reader_event::shutdown)
            return;
        }
        events.push(WS_reader_event::read_error)
        return;
    }
    unsigned int oldlen = data.size();
    data.resize(oldlen+read);
    buffer.sgetn(&data.front()+oldlen,read);
    start_reading();
}