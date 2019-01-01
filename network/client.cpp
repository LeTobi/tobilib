#include "client.h"

namespace tobilib::stream
{
	WS_Client::WS_Client(): rslv(ioc)
	{ }
	
	void WS_Client::intern_on_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::results_type results)
	{
		if (!_connecting)
			return;
		if (err)
		{
			Exception e (err.message());
			e.trace.push_back("WS_Client::intern_on_resolve()");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			_connecting = false;
			return;
		}
		boost::asio::async_connect(socket.next_layer(),results,boost::bind(&WS_Client::intern_on_connect,this,_1,_2));
	}
	
	void WS_Client::intern_on_connect(const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& ep)
	{
		if (!_connecting)
			return;
		if (ec)
		{
			Exception e (ec.message());
			e.trace.push_back("WS_Client::intern_on_connect()");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			_connecting = false;
			return;
		}
		socket.async_handshake(host,"/",boost::bind(&WS_Client::intern_on_handshake,this,_1));
	}
	
	void WS_Client::intern_on_handshake(boost::system::error_code const& ec)
	{
		if (!_connecting)
			return;
		if (ec)
		{
			Exception e (ec.message());
			e.trace.push_back("WS_Client::intern_on_handshake()");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			_connecting = false;
			close_tcp();
			return;
		}
		timeout.disable();
		_connecting = false;
		tick();
	}
	
	void WS_Client::connect(const std::string& _host, int _port)
	{
		if (_connecting || status()!=Status::Closed)
		{
			Exception e (std::string("Der Endpunkt ist nicht bereit fuer eine Verbindung mit ")+host+":"+std::to_string(port));
			e.trace.push_back("WS_Client::connect()");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			return;
		}
		timeout.set();
		host = _host;
		port = _port;
		boost::system::error_code err;
		_connecting = true;
		rslv.async_resolve(host,std::to_string(port),boost::bind(&WS_Client::intern_on_resolve,this,_1,_2));
	}

	void WS_Client::tick()
	{
		WS_Endpoint::tick();
		if (timeout.due())
		{
			timeout.disable();
			Exception e ("Verbindungstimeout");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			_connecting = false;
			close_tcp();
			return;
		}
	}

	WS_Client::Status WS_Client::status() const
	{
		if (_connecting)
			return Status::Connecting;
		switch (WS_Endpoint::status())
		{
			case WS_Endpoint::Status::Closed:
				return Status::Closed;
			case WS_Endpoint::Status::Active:
				return Status::Active;
			case WS_Endpoint::Status::Idle:
				return Status::Idle;
			case WS_Endpoint::Status::Shutdown:
				return Status::Shutdown;
		}
		throw Exception ("WS_Client::status() hat undefinierten Zustand!");
	}

	std::string WS_Client::mytrace() const
	{
		std::string out;
		out = "WS_Client";
		if (_connecting)
		{
			out+=" (connecting to ";
			out+= host + ":";
			out+= std::to_string(port);
			out+=")";
		}
		out+= ", ";
		out+= WS_Endpoint::mytrace();
		return out;
	}
}