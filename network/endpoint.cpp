#include "endpoint.h"

namespace tobilib::stream
{
	// Achtung, Hier ist noch ein Gefährlicher Zwischenstand!!!!!
	WS_Endpoint::WS_Endpoint(Process& proc): myprocess(proc), socket(myprocess), timer(myprocess)
	{
		myprocess.on_step.notify(boost::bind(&WS_Endpoint::tick,this));
	}

	void WS_Endpoint::intern_on_write(const boost::system::error_code& ec, size_t s)
	{
		_state &= ~EndpointFlags::writing;
		if (ec)
		{
			_state |= EndpointFlags::breakdown;
			on_error(network_error(std::string("WS_Endpoint::intern_on_write(): ")+ec.message()));
			return;
		}
		writebuffer.erase(0,s);
		flush();
		_state &= ~EndpointFlags::idle;
	}
	
	void WS_Endpoint::intern_on_read(const boost::system::error_code& ec, size_t s)
	{
		_state &= ~EndpointFlags::reading;
		if (ec)
		{
			close();
			if (ec.value() == (int)boost::beast::websocket::error::closed)
				return;
			on_error(network_error(std::string("WS_Endpoint::intern_on_read(): ")+ec.message()));
			return;
		}
		int oldlen = received.size();
		received.resize(oldlen+s);
		buffer.sgetn(&received.front()+oldlen,s);
		read();
		_state &= ~EndpointFlags::idle;
		on_receive();
	}
	
	void WS_Endpoint::intern_on_close(const boost::system::error_code& ec)
	{
		_state &= ~EndpointFlags::closing;
		if (ec)
		{
			boost::system::error_code err;
			socket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
			socket.next_layer().close();
			on_error(network_error(std::string("WS_Endpoint::intern_on_close() (Websocket close-Frame): ")+ec.message()));
		}
	}
	
	void WS_Endpoint::intern_on_timeout(const boost::system::error_code& ec)
	{
		_state &= ~EndpointFlags::timing;
		if (_status != EndpointStatus::open)
			return;
		if (ec)
		{
			on_error(network_error("Fehler in system_timer fuer Timeoutberechnung"));
			return;
		}
		if (_state & EndpointFlags::idle)
		{
			_state |= EndpointFlags::warned;
			on_timeout(_state & EndpointFlags::warned);
		}
		else
		{
			_state &= ~EndpointFlags::warned;
		}
		_state |= EndpointFlags::idle;
		timerset();
	}

	void WS_Endpoint::tick()
	{
		switch (_status)
		{
			case EndpointStatus::open:
				if (_state & EndpointFlags::breakdown)
				{
					_status = EndpointStatus::shutdown;
					break;
				}
				break;

			case EndpointStatus::shutdown:
				if (~_state & EndpointFlags::writing)
				{
					send_close();
					_status = EndpointStatus::closing;
				}
				break;

			case EndpointStatus::closing:
				if (~_state & EndpointFlags::reading)
				{
					_status = EndpointStatus::closed;
					_state &= ~EndpointFlags::breakdown;
					on_close();
					return;
				}
				break;

			case EndpointStatus::closed:
				if (_state & EndpointFlags::ready)
				{
					_status = EndpointStatus::open;
					_state &= ~EndpointFlags::ready;
					_state &= ~EndpointFlags::idle;
					read();
					timerset();
				}
				break;
		}
	}

	void WS_Endpoint::flush()
	{
		if (_status!=EndpointStatus::open && _status!=EndpointStatus::shutdown)
			return;
		if (_state & EndpointFlags::writing)
			return;
		writebuffer+=outqueue;
		outqueue.clear();
		if (writebuffer.size()==0)
			return;
		_state |= EndpointFlags::writing;
		socket.async_write(boost::asio::buffer(writebuffer),boost::bind(&WS_Endpoint::intern_on_write,this,_1,_2));
	}
	
	void WS_Endpoint::send_close()
	{
		_state |= EndpointFlags::closing;
		socket.async_close(boost::beast::websocket::close_reason("WS_Endpoint::close"),boost::bind(&WS_Endpoint::intern_on_close,this,_1));
	}

	void WS_Endpoint::read()
	{
		if (_state & EndpointFlags::writing)
			return;
		_state |= EndpointFlags::reading;
		socket.async_read(buffer,boost::bind(&WS_Endpoint::intern_on_read,this,_1,_2));
	}
	
	void WS_Endpoint::timerset()
	{
		if (_state & EndpointFlags::timing)
			return;
		_state |= EndpointFlags::timing;
		timer.expires_after(boost::asio::chrono::seconds(10));
		timer.async_wait(boost::bind(&WS_Endpoint::intern_on_timeout,this,_1));
	}

	void WS_Endpoint::start()
	{
		if (_status != EndpointStatus::closed)
			return;
		_state |= EndpointFlags::ready;
		tick();
	}
	
	void WS_Endpoint::write(const std::string& msg)
	{
		outqueue+=msg;
		flush();
	}
	
	void WS_Endpoint::close()
	{
		if (_status != EndpointStatus::open)
			return;
		_state |= EndpointFlags::breakdown;
	}
	
	bool WS_Endpoint::busy() const
	{
		return writing;
	}
}