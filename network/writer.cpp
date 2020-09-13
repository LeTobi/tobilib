#include "writer.h"
#include <boost/bind/bind.hpp>
#include "../general/exception.hpp"

using namespace tobilib;
using namespace network;
using namespace detail;
using namespace boost::placeholders;

template<class SocketType>
SocketWriter<SocketType>::SocketWriter(WriterOptions& _options, SocketType& _socket):
    options(_options),
    socket(_socket)
{ }

template<class SocketType>
WebsocketWriter<SocketType>::WebsocketWriter(WriterOptions& _options, WebsocketType& _socket):
    options(_options),
    socket(_socket)
{ }

template<class SocketType>
void SocketWriter<SocketType>::tick()
{
    if (timer.due())
    {
        timed_out = true;
        timer.disable();
    }
}

template<class SocketType>
void WebsocketWriter<SocketType>::tick()
{
    if (timer.due())
    {
        timed_out = true;
        timer.disable();
    }
}

template<class SocketType>
void SocketWriter<SocketType>::send_data(const std::string& msg)
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
    socket.async_write_some(
            boost::asio::buffer(data_sending),
            boost::bind(&SocketWriter<SocketType>::on_write,this,_1,_2)
        );
    async = true;
    if (options.send_timeout>0)
        timer.set(options.send_timeout);
}

template<class SocketType>
void WebsocketWriter<SocketType>::send_data(const std::string& msg)
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
    socket.async_write(
            boost::asio::buffer(data_sending),
            boost::bind(&WebsocketWriter<SocketType>::on_write,this,_1,_2)
        );
    async = true;
    if (options.send_timeout>0)
        timer.set(options.send_timeout);
}

template<class SocketType>
bool SocketWriter<SocketType>::is_async() const
{
    return async;
}

template<class SocketType>
bool WebsocketWriter<SocketType>::is_async() const
{
    return async;
}

template<class SocketType>
void SocketWriter<SocketType>::reset()
{
    if (async)
        throw Exception("Implementierungsfehler: Offener Schreibauftrag","SocketWriter::reset()");
    error.clear();
    timer.disable();
    timed_out = false;
    data_queue.clear();
    data_sending.clear();
}

template<class SocketType>
void WebsocketWriter<SocketType>::reset()
{
    if (async)
        throw Exception("Implementierungsfehler: Offener Schreibauftrag","WebsocketWriter::reset()");
    error.clear();
    timer.disable();
    timed_out = false;
    data_queue.clear();
    data_sending.clear();
}

template<class SocketType>
void SocketWriter<SocketType>::on_write(const boost::system::error_code& ec, std::size_t len)
{
    timer.disable();
    async = false;
    data_sending = data_sending.substr(len);
    error = ec;
    send_data("");
}

template<class SocketType>
void WebsocketWriter<SocketType>::on_write(const boost::system::error_code& ec, std::size_t len)
{
    timer.disable();
    async = false;
    data_sending = data_sending.substr(len);
    error = ec;
    send_data("");
}

#ifndef TC_SSL_IMPL_ONLY

    template class SocketWriter<boost::asio::ip::tcp::socket>;
    template class WebsocketWriter<boost::asio::ip::tcp::socket>;

#else

    template class SocketWriter<SSL_Socket>;
    template class WebsocketWriter<SSL_Socket>;

#endif