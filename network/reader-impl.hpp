#include "reader.h"
#include "../general/exception.hpp"
#include <boost/bind/bind.hpp>

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

template<class SocketType>
SocketReader<SocketType>::SocketReader(ReaderOptions& _options, SocketType* _socket):
    options(_options),
    socket(_socket)
{ }

template<class SocketType>
WebsocketReader<SocketType>::WebsocketReader(ReaderOptions& _options, WebsocketType* _socket):
    options(_options),
    socket(_socket)
{ }

template<class SocketType>
void SocketReader<SocketType>::tick()
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

template<class SocketType>
void WebsocketReader<SocketType>::tick()
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

template<class SocketType>
void SocketReader<SocketType>::start_reading()
{
    if (asio_reading)
        return;
    if (options.read_timeout>0)
        timer_B.set(options.read_timeout);
    if (options.inactive_warning>0)
        timer_A.set(options.inactive_warning);
    asio_reading = true;
    socket->async_read_some(
        boost::asio::buffer(buffer,BUFFER_SIZE),
        boost::bind(&SocketReader<SocketType>::on_receive,this,_1,_2)
    );
}

template<class SocketType>
void WebsocketReader<SocketType>::start_reading()
{
    if (asio_reading)
        return;
    if (options.read_timeout>0)
        timer_B.set(options.read_timeout);
    if (options.inactive_warning>0)
        timer_A.set(options.inactive_warning);
    asio_reading = true;
    socket->async_read(
        buffer,
        boost::bind(&WebsocketReader::on_receive,this,_1,_2)
    );
}

template<class SocketType>
void SocketReader<SocketType>::reset(SocketType* sock)
{
    asio_reading = false;
    socket = sock;
    warning = false;
    inactive = false;
    received = false;
    error.clear();
    data.clear();
    timer_A.disable();
    timer_B.disable();
}

template<class SocketType>
void WebsocketReader<SocketType>::reset(WebsocketType* sock)
{
    asio_reading = false;
    socket = sock;
    warning = false;
    inactive = false;
    received = false;
    error.clear();
    data.clear();
    timer_A.disable();
    timer_B.disable();
}

template<class SocketType>
void SocketReader<SocketType>::on_receive(const boost::system::error_code& ec, size_t recv_len)
{
    asio_reading = false;
    timer_A.disable();
    timer_B.disable();
    error = ec;
    data+=buffer.substr(0,recv_len);
    received = true;
    if (!ec)
        start_reading();
}

template<class SocketType>
void WebsocketReader<SocketType>::on_receive(const boost::system::error_code& ec, size_t recv_len)
{
    asio_reading = false;
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