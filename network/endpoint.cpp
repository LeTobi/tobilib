#include "endpoint.h"

namespace tobilib::stream
{
	WS_Endpoint::WS_Endpoint(): socket(ioc)
	{}

	void WS_Endpoint::tick()
	{
		ioc.poll_one();
		write_tick();
		read_tick();
		if (_status == Status::Closed)
		{
			if (socket.is_open())
			{
				_status = Status::Active;
				try {
					last_ip = socket.next_layer().remote_endpoint().address();
				} catch (std::exception& err) {
					Exception e (err.what());
					e.trace.push_back("tick() - IP auslesen");
					e.trace.push_back(mytrace());
					warnings.push_back(e);
				}
				read_reset();
				write_reset();
				read_begin();
			}
			return;
		}
		else if (_status==Status::Active)
		{
			if (read_warning_timer.due())
			{
				read_warning_timer.disable();
				_status = Status::Idle;
			}
		}
		if (_readstatus==ReadStatus::Error || _writestatus==WriteStatus::Error)
		{
			close_tcp();
			return;
		}
	}

	WS_Endpoint::Status WS_Endpoint::status() const
	{
		return _status;
	}

	void WS_Endpoint::reactivate(const std::string& msg)
	{
		if (_status != Status::Idle)
			return;
		reactivate();
		write(msg);
	}

	void WS_Endpoint::shutdown()
	{
		if (_status != Status::Closed)
			_status = Status::Shutdown;
	}

	const boost::asio::ip::address& WS_Endpoint::remote_ip() const
	{
		return last_ip;
	}

	std::string WS_Endpoint::mytrace() const
	{
		std::string out = "WS_Endpoint ";
		switch (_status)
		{
			case Status::Active:
				out+="Aktiv";
				break;
			case Status::Idle:
				out+="Idle";
				break;
			case Status::Shutdown:
				out+="Shutdown";
				break;
			case Status::Closed:
				out+="Closed";
		}
		if (!last_ip.is_unspecified())
			out+=" remote-ip: "+last_ip.to_string();
		return out;
	}

	void WS_Endpoint::close_tcp()
	{
		_status = Status::Closed;
		if (!socket.next_layer().is_open())
			return;
		boost::system::error_code err;
		socket.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
		if (err)
		{
			Exception e (err.message());
			e.trace.push_back("close_tcp()");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
		}
		socket.next_layer().close();
	}

	void WS_Endpoint::reactivate()
	{
		if (_status == Status::Idle)
			_status = Status::Active;
	}

	void WS_Endpoint::write_begin()
	{
		if (_writestatus != WriteStatus::Idle)
			return;
		write_buffer+=write_queue;
		write_queue.clear();
		if (write_buffer.size()==0)
			return;
		_writestatus = WriteStatus::Msg;
		socket.async_write(boost::asio::buffer(write_buffer),boost::bind(&WS_Endpoint::write_end,this,_1,_2));
	}

	void WS_Endpoint::write_end(const boost::system::error_code& ec, size_t s)
	{
		if (ec)
		{
			_writestatus = WriteStatus::Error;
			if (_status != Status::Closed)
			{
				Exception e (ec.message());
				e.trace.push_back("write_end()");
				e.trace.push_back(mytrace());
				warnings.push_back(e);
			}
			close_tcp();
			return;
		}
		_writestatus = WriteStatus::Idle;
		write_buffer.erase(0,s);
		write_begin();
	}

	void WS_Endpoint::write_close_begin()
	{
		if (_writestatus != WriteStatus::Idle)
			return;
		_writestatus = WriteStatus::Shutdown;
		write_close_timeout.set();
		socket.async_close(boost::beast::websocket::close_reason("WS_Endpoint::close"),boost::bind(&WS_Endpoint::write_close_end,this,_1));
	}

	void WS_Endpoint::write_close_end(const boost::system::error_code& ec)
	{
		write_close_timeout.disable();
		if (ec)
		{
			_writestatus = WriteStatus::Error;
			if (_status != Status::Closed)
			{
				Exception e (ec.message());
				e.trace.push_back("intern_on_close() (Websocket close-Frame)");
				e.trace.push_back(mytrace());
				warnings.push_back(e);
			}
			return;
		}
		_writestatus = WriteStatus::Terminated;
	}

	void WS_Endpoint::write_reset()
	{
		if (_writestatus == WriteStatus::Error || _writestatus == WriteStatus::Terminated)
			_writestatus = WriteStatus::Idle;
		write_queue.clear();	
	}

	void WS_Endpoint::write_tick()
	{
		if (write_close_timeout.due())
		{
			write_close_timeout.disable();
			_writestatus = WriteStatus::Error;
		}
		if (_writestatus == WriteStatus::Idle)
		{
			if (_status == Status::Shutdown)
				write_close_begin();
		}
	}

	bool WS_Endpoint::write_busy() const
	{
		return _writestatus != WriteStatus::Idle;
	}

	void WS_Endpoint::write(const std::string& msg)
	{
		write_queue+=msg;
		write_begin();
	}

	void WS_Endpoint::read_begin()
	{
		if (_readstatus != ReadStatus::Idle)
			return;
		read_warning_timer.set();
		read_shutdown_timer.set();
		_readstatus = ReadStatus::Reading;
		socket.async_read(read_buffer,boost::bind(&WS_Endpoint::read_end,this,_1,_2));
	}

	void WS_Endpoint::read_end(const boost::system::error_code& ec, size_t s)
	{
		read_warning_timer.disable();
		read_shutdown_timer.disable();
		reactivate();
		if (ec)
		{
			_readstatus = ReadStatus::Error;
			if (_status!=Status::Closed && _writestatus!=WriteStatus::Terminated && ec.value()!=(int)boost::beast::websocket::error::closed)
			{
				Exception e (ec.message());
				e.trace.push_back("intern_on_read()");
				e.trace.push_back(mytrace());
				warnings.push_back(e);
			}
			return;
		}
		_readstatus = ReadStatus::Idle;
		int oldlen = read_data.size();
		read_data.resize(oldlen+s);
		read_buffer.sgetn(&read_data.front()+oldlen,s);
		read_begin();
	}

	void WS_Endpoint::read_reset()
	{
		if (_readstatus == ReadStatus::Error)
			_readstatus = ReadStatus::Idle;
		read_data.clear();
	}

	void WS_Endpoint::read_tick()
	{
		if (read_shutdown_timer.due())
		{
			read_shutdown_timer.disable();
			Exception e ("Endpunkt inaktiv: Verbindung abbrechen");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			_readstatus = ReadStatus::Error;
		}
	}

	int WS_Endpoint::read_size() const
	{
		return read_data.size();
	}

	std::string WS_Endpoint::read(unsigned int len)
	{
		if (len==0)
		{
			std::string out = read_data;
			read_data.clear();
			return out;
		}
		else
		{
			if (read_data.size()<len)
				return "";
			std::string out = read_data.substr(0,len);
			read_data.erase(0,len);
			return out;
		}
	}
}