#include "endpoint.h"
#include <boost/bind.hpp>

using namespace tobilib;
using namespace network;
using namespace detail;

WS_Endpoint::WS_Endpoint():
    socket(ioc),
    reader(socket,options),
    writer(socket,options)
{ }

bool WS_Endpoint::is_inactive() const
{
    return reader.is_inactive();
}

void WS_Endpoint::write(const std::string& msg)
{
    if (!is_connected())
        throw Exception("Implementierungsfehler: write() ohne Verbindung","WS_Endpoint::write()");
    writer.send_data(msg);
}

std::string WS_Endpoint::read(unsigned int len)
{
    if (len==0)
        len = reader.data.size();
    std::string out = reader.data.substr(0,len);
    reader.data = reader.data.substr(len);
    return out;
}

std::string WS_Endpoint::peek(unsigned int len) const
{
    if (len==0)
        len = reader.data.size();
    return reader.data.substr(0,len);
}

unsigned int WS_Endpoint::recv_size() const
{
    return reader.data.size();
}

void WS_Endpoint::close()
{
    if (!is_connected())
        throw Exception("Implementierungsfehler: close() ohne Verbindung","WS_Endpoint::close()");
    set_closing();
    writer.send_close();
}

void WS_Endpoint::ws_endpoint_tick()
{
    ioc.poll_one();
    writer.tick();
    reader.tick();
    if (!is_good())
        return;
    while (!writer.events.empty())
        switch(writer.events.next())
        {
        case WS_writer_event::send_timeout:
            log << "send_timeout" << std::endl;
            abort();
            break;
        case WS_writer_event::close_timeout:
            log << "close_timeout" << std::endl;
            abort();
            break;
        case WS_writer_event::send_error:
            fail("error sending data");
            break;
        }
    while (!reader.events.empty())
        switch(reader.events.next())
        {
        case WS_reader_event::data_arrived:
            events.push(Event::read);
            break;
        case WS_reader_event::inactive_warning:
            events.push(Event::inactive);
            break;
        case WS_reader_event::inactive_shutdown:
            log << "inactive connection" << std::endl;
            close();
            break;
        case WS_reader_event::read_error:
            socket.next_layer().close();
            if (status()==Status::closing)
                set_closed();
            else
                fail("reading error");
            break;
        case WS_reader_event::shutdown:
            socket.next_layer().close();
            set_closed();
            break;
        }
}

void WS_Endpoint::ws_endpoint_abort()
{
    socket.next_layer().close();
    while (reader.is_reading() || writer.is_busy())
        ioc.poll_one();
    reader.events.clear();
    writer.events.clear();
}

void WS_Endpoint::ws_endpoint_reset()
{
    reader.reset();
    writer.reset();
}

void WS_Endpoint::start_reading()
{
    if (status() != Status::connecting)
        throw Exception("Implementierungsfehler: start_reading()","WS_Endpoint::start_reading()");
    set_connected();
    reader.start_reading();
}