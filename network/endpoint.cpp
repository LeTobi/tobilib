#include "endpoint.h"

namespace tobilib::stream
{
	WS_Endpoint::WS_Endpoint(): socket(ioc)
	{}

	void WS_Endpoint::timeout_reset()
	{
		time(&last_interaction);
		_state &= ~Flags::informed;
	}

	void WS_Endpoint::intern_on_write(const boost::system::error_code& ec, size_t s)
	{
		_state &= ~Flags::writing;
		if (ec)
		{
			_state |= Flags::breakdown;
			Exception e (ec.message());
			e.trace.push_back("intern_on_write()");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			return;
		}
		writebuffer.erase(0,s);
		flush();
	}
	
	void WS_Endpoint::intern_on_read(const boost::system::error_code& ec, size_t s)
	{
		_state &= ~Flags::reading;
		if (ec)
		{
			close();
			if (_status == EndpointStatus::closing)
				return;
			Exception e (ec.message());
			e.trace.push_back("intern_on_read()");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			return;
		}
		timeout_reset();
		int oldlen = received.size();
		received.resize(oldlen+s);
		buffer.sgetn(&received.front()+oldlen,s);
		intern_read();
	}
	
	void WS_Endpoint::intern_on_close(const boost::system::error_code& ec)
	{
		_state &= ~Flags::closing;
		if (ec)
		{
			close_socket();
			Exception e (ec.message());
			e.trace.push_back("intern_on_close() (Websocket close-Frame)");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
		}
	}

	void WS_Endpoint::close_socket()
	{
		disconnect_time = 0;
		boost::system::error_code err;
		socket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
		if (err)
		{
			Exception e (err.message());
			e.trace.push_back("WS_Endpoint::close_socket()");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
		}
		socket.next_layer().close();
	}

	void WS_Endpoint::tick()
	{
		ioc.poll_one();
		time_t now;
		time(&now);
		if (disconnect_time>0 && difftime(now,disconnect_time)>5)
		{
			Exception e ("Verbindungsabbruch wird erzwungen...");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			close_socket();
		}
		switch (_status)
		{
			case EndpointStatus::open:
				if (difftime(now,last_interaction)>20)
				{
					Exception e ("Timeout erreicht. Verbindung inaktiv");
					e.trace.push_back(mytrace());
					warnings.push_back(e);
					_state |= Flags::breakdown;
				}
				if (_state & Flags::breakdown)
				{
					_status = EndpointStatus::shutdown;
					time(&disconnect_time);
					break;
				}
				break;

			case EndpointStatus::shutdown:
				if (~_state & Flags::writing)
				{
					send_close();
					_status = EndpointStatus::closing;
				}
				break;

			case EndpointStatus::closing:
				if (~_state & Flags::reading)
				{
					_status = EndpointStatus::closed;
					_state &= ~Flags::breakdown;
					timeout_reset();
					return;
				}
				break;

			case EndpointStatus::closed:
				if (_state & Flags::ready)
				{
					_status = EndpointStatus::open;
					_state &= ~Flags::ready;
					intern_read();
				}
				break;
		}
	}

	void WS_Endpoint::flush()
	{
		if (_status!=EndpointStatus::open && _status!=EndpointStatus::shutdown)
			return;
		if (_state & Flags::writing)
			return;
		writebuffer+=outqueue;
		outqueue.clear();
		if (writebuffer.size()==0)
			return;
		_state |= Flags::writing;
		socket.async_write(boost::asio::buffer(writebuffer),boost::bind(&WS_Endpoint::intern_on_write,this,_1,_2));
	}
	
	void WS_Endpoint::send_close()
	{
		_state |= Flags::closing;
		socket.async_close(boost::beast::websocket::close_reason("WS_Endpoint::close"),boost::bind(&WS_Endpoint::intern_on_close,this,_1));
	}

	void WS_Endpoint::intern_read()
	{
		if (_state & Flags::reading)
			return;
		_state |= Flags::reading;
		socket.async_read(buffer,boost::bind(&WS_Endpoint::intern_on_read,this,_1,_2));
	}

	void WS_Endpoint::start()
	{
		if (_status != EndpointStatus::closed)
			return;
		_state |= Flags::ready;
		try
		{
			last_ip = socket.next_layer().remote_endpoint().address();
		}
		catch (std::exception& err)
		{
			Exception e (err.what());
			e.trace.push_back("WS_Endpoint::start(), Zuweisung von last_ip");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
		}
	}
	
	void WS_Endpoint::write(const std::string& msg)
	{
		outqueue+=msg;
		flush();
	}
	
	bool WS_Endpoint::readable(unsigned int len) const
	{
		return received.size()>=len;
	}

	std::string WS_Endpoint::read(unsigned int len)
	{
		if (len==0)
		{
			std::string out = received;
			received.clear();
			return out;
		}
		else
		{
			if (received.size()<len)
				return "";
			std::string out = received.substr(0,len);
			received.erase(0,len);
			return out;
		}
	}

	bool WS_Endpoint::inactive() const
	{
		time_t now;
		time(&now);
		return difftime(now,last_interaction)>=10 && _status==EndpointStatus::open && (~_state & Flags::informed);
	}

	void WS_Endpoint::inactive_checked()
	{
		if (inactive())
			_state |= Flags::informed;
	}

	void WS_Endpoint::close()
	{
		if (_status != EndpointStatus::open)
			return;
		_state |= Flags::breakdown;
	}
	
	bool WS_Endpoint::busy() const
	{
		return _state & Flags::writing;
	}

	const boost::asio::ip::address& WS_Endpoint::remote_ip() const
	{
		return last_ip;
	}

	std::string WS_Endpoint::mytrace() const
	{
		std::string out = "WS_Endpoint, ";
		switch (_status)
		{
			case EndpointStatus::open:
				out+="connected";
				break;
			case EndpointStatus::shutdown:
				out+="preparing disconnection";
				break;
			case EndpointStatus::closing:
				out+="disconnecting";
				break;
			case EndpointStatus::closed:
				out+="disconnected";
		}
		if (_status==EndpointStatus::closed)
		{
			if (!last_ip.is_unspecified())
				out+=", last ip: "+last_ip.to_string();
		}
		else
		{
			out+=" "+last_ip.to_string();
		}
		return out;
	}
}