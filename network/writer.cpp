#include "writer.h"
#include <boost/bind/bind.hpp>
#include "../general/exception.hpp"

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

template<class Stream>
StreamWriter<Stream>::StreamWriter(WriterOptions& _options, Stream& _stream):
    options(_options),
    stream(_stream)
{ }

template<class Stream>
WS_Writer<Stream>::WS_Writer(WriterOptions& _options, WSStream& _stream):
    options(_options),
    stream(_stream)
{ }

template<class Stream>
void StreamWriter<Stream>::tick()
{
    if (timer.due())
    {
        timed_out = true;
        timer.disable();
    }
}

template<class Stream>
void WS_Writer<Stream>::tick()
{
    if (timer.due())
    {
        timed_out = true;
        timer.disable();
    }
}

template<class Stream>
void StreamWriter<Stream>::send_data(const std::string& msg)
{
    data_queue += msg;
    if (async)
        return;
    data_sending += data_queue;
    data_queue.clear();
    if (data_sending.empty())
    {
        written = true;
        return;
    }
    stream.async_write_some(
            boost::asio::buffer(data_sending),
            boost::bind(&StreamWriter<Stream>::on_write,this,_1,_2)
        );
    async = true;
    if (options.send_timeout>0)
        timer.set(options.send_timeout);
}

template<class Stream>
void WS_Writer<Stream>::send_data(const std::string& msg)
{
    data_queue += msg;
    if (async)
        return;
    data_sending += data_queue;
    data_queue.clear();
    if (data_sending.empty())
    {
        written = true;
        return;
    }
    stream.async_write(
            boost::asio::buffer(data_sending),
            boost::bind(&WS_Writer<Stream>::on_write,this,_1,_2)
        );
    async = true;
    if (options.send_timeout>0)
        timer.set(options.send_timeout);
}

template<class Stream>
bool StreamWriter<Stream>::is_async() const
{
    return async;
}

template<class Stream>
bool WS_Writer<Stream>::is_async() const
{
    return async;
}

template<class Stream>
void StreamWriter<Stream>::reset()
{
    if (async)
        throw Exception("Implementierungsfehler: Offener Schreibauftrag","StreamWriter::reset()");
    error.clear();
    timer.disable();
    timed_out = false;
    data_queue.clear();
    data_sending.clear();
}

template<class Stream>
void WS_Writer<Stream>::reset()
{
    if (async)
        throw Exception("Implementierungsfehler: Offener Schreibauftrag","WS_Writer::reset()");
    error.clear();
    timer.disable();
    timed_out = false;
    data_queue.clear();
    data_sending.clear();
}

template<class Stream>
void StreamWriter<Stream>::on_write(const boost::system::error_code& ec, std::size_t len)
{
    timer.disable();
    async = false;
    data_sending = data_sending.substr(len);
    error = ec;
    send_data("");
}

template<class Stream>
void WS_Writer<Stream>::on_write(const boost::system::error_code& ec, std::size_t len)
{
    timer.disable();
    async = false;
    data_sending = data_sending.substr(len);
    error = ec;
    send_data("");
}

#ifndef TC_SSL_IMPL_ONLY

    template class StreamWriter<boost::asio::ip::tcp::socket>;
    template class WS_Writer<boost::asio::ip::tcp::socket>;

#else

    template class StreamWriter<SSL_Stream>;
    template class WS_Writer<SSL_Stream>;

#endif