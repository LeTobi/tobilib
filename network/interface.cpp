#include "interface.h"
#include "acceptor.h"

#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

using namespace tobilib;
using namespace network;
using namespace detail;
using boost::placeholders::_1;
using boost::placeholders::_2;

template<class StackConfig>
Endpoint<StackConfig>::Endpoint(Acceptor& _acceptor):
    socket(ioc),
    reader(options,socket),
    writer(options,socket),
    closer(socket,ioc,log)
{
    connector = new typename StackConfig::ServerConnector(_acceptor,socket,ioc,options);
}

template<class StackConfig>
Endpoint<StackConfig>::Endpoint(const std::string& _address, unsigned int _port):
    socket(ioc),
    reader(options,socket),
    writer(options,socket),
    closer(socket,ioc,log)
{
    connector = new typename StackConfig::ClientConnector(_address,_port,socket,ioc,options);
}

template<class StackConfig>
Endpoint<StackConfig>::~Endpoint()
{
    delete connector;
}

template<class StackConfig>
EndpointStatus Endpoint<StackConfig>::status() const
{
    return _status;
}

template<class StackConfig>
bool Endpoint<StackConfig>::is_connected() const
{
    return _status == EndpointStatus::connected;
}

template<class StackConfig>
bool Endpoint<StackConfig>::is_closed() const
{
    return _status == EndpointStatus::closed;
}

template<class StackConfig>
boost::asio::ip::address Endpoint<StackConfig>::remote_ip() const
{
    return _remote_ip;
}

template<class StackConfig>
void Endpoint<StackConfig>::tick()
{
    ioc.poll_one();

    reader.tick();
    if (reader.warning)
    {
        reader.warning = false;
        events.push(EndpointEvent::inactive);
    }
    if (reader.inactive)
    {
        reader.inactive = false;
        log << "Inaktiver Endpunkt" << std::endl;
        close();
    }
    if (reader.error)
    {
        reader.error.clear();
        reset();
    }
    if (reader.received)
    {
        reader.received = false;
        events.push(EndpointEvent::received);
    }

    writer.tick();
    if (writer.timed_out)
    {
        writer.timed_out = false;
        log << "Zeitueberschreitung beim Schreibvorgang" << std::endl;
        close();
    }
    if (writer.error)
    {
        log << "Schreibfehler: " << writer.error.message() << std::endl;
        writer.error.clear();
        close();
    }
    if (writer.written)
    {
        writer.written=false;
        if (_status==EndpointStatus::closing)
            closer.close();
    }

    connector->tick();
    if (connector->finished)
    {
        connector->finished = false;
        connect_timer.disable();
        if (connector->error)
        {
            log << connector->error.message() << std::endl;
            reset();
        }
        else
        {
            set_connected();
            _remote_ip = connector->remote_ip;
            reader.start_reading();
        }
    }
    
    if (connect_timer.due())
    {
        connect_timer.disable();
        log << "Zeitueberschreitung bei Verbindungsvorgang" << std::endl;
        close();
    }
    if (close_timer.due())
    {
        close_timer.disable();
        log << "Zeitueberschreitung bei Schliessvorgang" << std::endl;
        reset();
    }
}

template<class StackConfig>
void Endpoint<StackConfig>::connect()
{
    if (_status != EndpointStatus::closed)
        throw Exception("connect() in falschem Zustand","Endpoint::connect()");
    if (options.connect_timeout>0)
        connect_timer.set(options.connect_timeout);
    connector->connect();
    set_connecting();
}

template<class StackConfig>
void Endpoint<StackConfig>::write(const std::string& msg)
{
    if (_status != EndpointStatus::connected)
        throw Exception("Schreiben in ungueltigem Zustand","Endpoint::write()");
    writer.send_data(msg);
}

template<class StackConfig>
std::string Endpoint<StackConfig>::read(unsigned int len)
{
    if (len==0)
        len = reader.data.size();
    std::string out = reader.data.substr(0,len);
    reader.data = reader.data.substr(len);
    return out;
}

template<class StackConfig>
std::string Endpoint<StackConfig>::peek(unsigned int len) const
{
    if (len==0)
        len = reader.data.size();
    return reader.data.substr(0,len);
}

template<class StackConfig>
unsigned int Endpoint<StackConfig>::recv_size() const
{
    return reader.data.size();
}

template<class StackConfig>
void Endpoint<StackConfig>::close()
{
    if (_status==EndpointStatus::closing)
        return;
    if (_status!=EndpointStatus::connected)
    {
        reset();
        return;
    }
    if (options.close_timeout>0)
        close_timer.set(options.close_timeout);
    set_closing();
    if (!writer.is_busy())
        closer.close();
}

template<class StackConfig>
void Endpoint<StackConfig>::reset()
{
    if (_status==EndpointStatus::closed)
        return;
    connector->reset();
    connect_timer.disable();
    close_timer.disable();
    closer.reset();
    while (reader.is_reading() || writer.is_busy())
        ioc.poll_one();
    reader.reset();
    writer.reset();
    set_closed();
}

template<class StackConfig>
void Endpoint<StackConfig>::set_connecting()
{
    _status = EndpointStatus::connecting;
}

template<class StackConfig>
void Endpoint<StackConfig>::set_connected()
{
    if (_status != EndpointStatus::connected)
        events.push(EndpointEvent::connected);
    _status = EndpointStatus::connected;
}

template<class StackConfig>
void Endpoint<StackConfig>::set_closing()
{
    _status = EndpointStatus::closing;
}

template<class StackConfig>
void Endpoint<StackConfig>::set_closed()
{
    if (_status != EndpointStatus::closed)
        events.push(EndpointEvent::closed);
    _status = EndpointStatus::closed;
}

template class Endpoint<Config_TCP>;
template class Endpoint<Config_WS>;