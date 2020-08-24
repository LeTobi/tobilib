#include "reader.h"
#include "../general/exception.hpp"
#include <boost/bind/bind.hpp>

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

template<class Stream>
StreamReader<Stream>::StreamReader(ReaderOptions& _options, Stream& _stream):
    options(_options),
    stream(_stream)
{ }

template<class Stream>
WS_Reader<Stream>::WS_Reader(ReaderOptions& _options, WSStream& _stream):
    options(_options),
    stream(_stream)
{ }

template<class Stream>
void StreamReader<Stream>::tick()
{
    if (timer_A.due())
    {
        timer_A.disable();
        warning = true;
    }
    if (timer_B.due())
    {
        timer_B.disable();
        inactive = true;
    }
}

template<class Stream>
void WS_Reader<Stream>::tick()
{
    if (timer_A.due())
    {
        timer_A.disable();
        warning = true;
    }
    if (timer_B.due())
    {
        timer_B.disable();
        inactive = true;
    }
}

template<class Stream>
void StreamReader<Stream>::start_reading()
{
    if (async)
        return;
    if (options.read_timeout>0)
        timer_B.set(options.read_timeout);
    if (options.inactive_warning>0)
        timer_A.set(options.inactive_warning);
    async = true;
    stream.async_read_some(
        boost::asio::buffer(buffer,BUFFER_SIZE),
        boost::bind(&StreamReader<Stream>::on_receive,this,_1,_2)
    );
}

template<class Stream>
void WS_Reader<Stream>::start_reading()
{
    if (async)
        return;
    if (options.read_timeout>0)
        timer_B.set(options.read_timeout);
    if (options.inactive_warning>0)
        timer_A.set(options.inactive_warning);
    async = true;
    stream.async_read(
        buffer,
        boost::bind(&WS_Reader::on_receive,this,_1,_2)
    );
}

template<class Stream>
bool StreamReader<Stream>::is_async() const
{
    return async;
}

template<class Stream>
bool WS_Reader<Stream>::is_async() const
{
    return async;
}

template<class Stream>
void StreamReader<Stream>::reset()
{
    if (async)
        throw Exception("Implementierungsfehler: Offene Leseanfrage","StreamReader::reset()");
    warning = false;
    inactive = false;
    received = false;
    error.clear();
    data.clear();
    timer_A.disable();
    timer_B.disable();
}

template<class Stream>
void WS_Reader<Stream>::reset()
{
    if (async)
        throw Exception("Implementierungsfehler: Offene Leseanfrage","WS_Reader::reset()");
    warning = false;
    inactive = false;
    received = false;
    error.clear();
    data.clear();
    timer_A.disable();
    timer_B.disable();
}

template<class Stream>
void StreamReader<Stream>::on_receive(const boost::system::error_code& ec, size_t recv_len)
{
    async = false;
    timer_A.disable();
    timer_B.disable();
    error = ec;
    data+=buffer.substr(0,recv_len);
    received = true;
    if (!ec)
        start_reading();
}

template<class Stream>
void WS_Reader<Stream>::on_receive(const boost::system::error_code& ec, size_t recv_len)
{
    async = false;
    timer_A.disable();
    timer_B.disable();
    error = ec;
    unsigned int oldlen = data.size();
    data.resize(oldlen+recv_len);
    buffer.sgetn(&data.front()+oldlen,recv_len);
    received=true;
    if (!ec)
        start_reading();
}

#ifndef TC_SSL_IMPL_ONLY

    template class StreamReader<boost::asio::ip::tcp::socket>;
    template class WS_Reader<boost::asio::ip::tcp::socket>;

#else

    template class StreamReader<SSL_Stream>;
    template class WS_Reader<SSL_Stream>;

#endif