#include "client.h"

namespace tobilib::stream
{
	WS_Client::WS_Client(): rslv(ioc)
	{ }
	
	void WS_Client::intern_on_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::results_type results)
	{
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
		if (ec)
		{
			Exception e (ec.message());
			e.trace.push_back("WS_Client::intern_on_handshake()");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			close_socket();
			_connecting = false;
			return;
		}
		_connecting = false;
		start();
		tick();
	}
	
	void WS_Client::connect(const std::string& _host, int _port)
	{
		if (_connecting || status()!=EndpointStatus::closed)
		{
			Exception e (std::string("Der Endpunkt ist nicht bereit fuer eine Verbindung mit ")+host+":"+std::to_string(port));
			e.trace.push_back("WS_Client::connect()");
			e.trace.push_back(mytrace());
			return;
		}
		host = _host;
		port = _port;
		boost::system::error_code err;
		_connecting = true;
		rslv.async_resolve(host,std::to_string(port),boost::bind(&WS_Client::intern_on_resolve,this,_1,_2));
	}

	bool WS_Client::connecting() const
	{
		return _connecting;
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