#include "endpoint.h"

namespace tobilib::stream
{
	void WS_Endpoint::intern_on_write(const boost::system::error_code& ec, size_t s)
	{
		writing = false;
		if (ec)
		{
			on_error(network_error(std::string("WS_Endpoint::intern_on_write(): ")+ec.message()));
			close();
			return;
		}
		if (_status != EndpointState::open)
		{
			close();
			return;
		}
		writebuffer.erase(0,s);
		flush();
	}
	
	void WS_Endpoint::intern_on_read(const boost::system::error_code& ec, size_t s)
	{
		reading = false;
		if (ec)
		{
			if (ec.value() == (int)boost::beast::websocket::error::closed)
			{
				_status = EndpointState::closed;
				close();
				return;
			}
			on_error(network_error(std::string("WS_Endpoint::intern_on_read(): ")+ec.message()));
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
		if (ec)
		{
			on_error(network_error(std::string("WS_Endpoint::intern_on_close() (Websocket close-Frame): ")+ec.message()));
			boost::system::error_code err;
			socket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
			socket.next_layer().close();
		}
		if (!reading)
		{
			_status = EndpointState::closed;
		}
		close();
	}
	
	void WS_Endpoint::flush()
	{
		if (_status != EndpointState::open)
			return;
		if (writing)
			return;
		writebuffer+=outqueue;
		outqueue.clear();
		if (writebuffer.size()==0)
			return;
		writing = true;
		socket.async_write(boost::asio::buffer(writebuffer),boost::bind(&WS_Endpoint::intern_on_write,this,_1,_2));
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
		{
			if ((!writing) && (!reading))
			{
				closing = false;
				on_close();
			}
			return;
		}
		_status = EndpointState::shutdown;
		if (writing || closing)
			return;
		closing = true;
		socket.async_close(boost::beast::websocket::close_reason("WS_Endpoint::close"),boost::bind(&WS_Endpoint::intern_on_close,this,_1));
	}
	
	bool WS_Endpoint::busy() const
	{
		return writing;
	}
}