#include "endpoint.h"

namespace tobilib::stream
{
	Endpoint_relay::Endpoint_relay(Endpoint& ep): origin(ep)
	{
		status_update();
		received = origin.received;
		origin.received.clear();
		origin.on_close.notify(std::bind(&Endpoint_relay::intern_close,this));
		origin.on_receive.notify(std::bind(&Endpoint_relay::intern_receive,this));
		origin.on_error.notify(std::bind(&Endpoint_relay::intern_error,this,std::placeholders::_1));
	}
	
	void Endpoint_relay::intern_receive()
	{
		received = origin.received;
		origin.received.clear();
		status_update();
		if (master_on_receive)
			master_on_receive();
		else
			on_receive();
	}
	
	void Endpoint_relay::intern_close()
	{
		status_update();
		if (master_on_close)
			master_on_close();
		else
			on_close();
	}
	
	void Endpoint_relay::intern_error(const network_error& err)
	{
		status_update();
		if (master_on_error)
			master_on_error(err);
		else
			on_error(err);
	}
	
	void WS_Endpoint::intern_on_write(const boost::system::error_code& ec, size_t s)
	{
		writing = false;
		if (ec)
		{
			on_error(network_error(std::string("WS_Endpoint::intern_on_write(): ")+ec.message()));
			close();
			return;
		}
		if (_status == EndpointState::shutdown)
		{
			close();
			return;
		}
		outqueue.erase(0,s);
		flush();
	}
	
	void WS_Endpoint::intern_on_read(const boost::system::error_code& ec, size_t s)
	{
		reading = false;
		if (ec)
		{
			if (ec.value() == (int)boost::beast::websocket::error::closed)
			{
				if (writing)
					return;
				_status = EndpointState::closed;
				on_close();
				return;
			}
			on_error(network_error(std::string("WS_Endpoint::intern_on_read(): ")+ec.message()));
			close();
			return;
		}
		if (_status == EndpointState::shutdown)
		{
			close();
			return;
		}
		int oldlen = received.size();
		received.resize(oldlen+s);
		buffer.sgetn(&received.front()+oldlen,s);
		on_receive();
		read();
	}
	
	void WS_Endpoint::intern_on_close(const boost::system::error_code& ec)
	{
		writing = false;
		if (ec)
		{
			on_error(network_error(std::string("WS_Endpoint::intern_on_close() (Websocket close-Frame): ")+ec.message()));
			boost::system::error_code err;
			socket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
			socket.next_layer().close();
		}
		if (reading)
			return;
		_status = EndpointState::closed;
		on_close();
	}
	
	void WS_Endpoint::flush()
	{
		if (outqueue.size()==0 || writing)
			return;
		writing = true;
		socket.async_write(boost::asio::buffer(outqueue.data(),outqueue.size()),boost::bind(&WS_Endpoint::intern_on_write,this,_1,_2));
	}
	
	void WS_Endpoint::read()
	{
		if (reading)
			return;
		reading = true;
		socket.async_read(buffer,boost::bind(&WS_Endpoint::intern_on_read,this,_1,_2));
	}
	
	void WS_Endpoint::start()
	{
		if (_status != EndpointState::closed)
			return;
		_status = EndpointState::open;
		read();
	}
	
	void WS_Endpoint::write(const std::string& msg)
	{
		outqueue+=msg;
		flush();
	}
	
	void WS_Endpoint::close()
	{
		if (_status == EndpointState::closed)
			return;
		_status = EndpointState::shutdown;
		if (writing)
		{
			return;
		}
		writing = true;
		socket.async_close(boost::beast::websocket::close_reason("WS_Endpoint::close"),boost::bind(&WS_Endpoint::intern_on_close,this,_1));
	}
}