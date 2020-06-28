#include "reader.h"
#include "../../general/exception.hpp"
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

using namespace tobilib;
using namespace detail;
using boost::placeholders::_1;
using boost::placeholders::_2;

WS_Reader::WS_Reader(boost::beast::websocket::stream<boost::asio::ip::tcp::socket>& _socket, WS_reader_options& _options):
    socket(_socket),
    options(_options)
{ }

void WS_Reader::tick()
{
    if (inactive_timer.due())
    {
        inactive_timer.disable();
        inactive = true;
        events.push(WS_reader_event::inactive_warning);
    }
    if (deadline_timer.due())
    {
        deadline_timer.disable();
        events.push(WS_reader_event::inactive_shutdown);
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
        inactive_timer.set(options.inactive_warning);
    if (options.inactive_shutdown>0)
        deadline_timer.set(options.inactive_shutdown);
    reading = true;
    socket.async_read(
        buffer,
        boost::bind(&WS_Reader::on_data_read,this,_1,_2)
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
    events.clear();
}

void WS_Reader::on_data_read(const boost::system::error_code& ec, std::size_t read)
{
    reading = false;
    inactive = false;
    inactive_timer.disable();
    deadline_timer.disable();
    if (ec)
    {
        if (ec.value() == (int)boost::beast::websocket::error::closed) {
            events.push(WS_reader_event::shutdown);
            return;
        }
        events.push(WS_reader_event::read_error);
        return;
    }
    events.push(WS_reader_event::data_arrived);
    unsigned int oldlen = data.size();
    data.resize(oldlen+read);
    buffer.sgetn(&data.front()+oldlen,read);
    start_reading();
}