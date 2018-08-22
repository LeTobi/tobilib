#include "client.h"

namespace tobilib::stream
{
	void WS_Client::intern_on_connect(const boost::system::error_code& ec, const boost::asio::ip::tcp::endpoint& ep)
	{
		if (ec)
		{
			on_error(network_error(std::string("WS_Client::intern_on_connect(): ")+ec.message()));
			return;
		}
		socket.async_handshake(host,"/",boost::bind(&WS_Client::intern_on_handshake,this,_1));
	}
	
	void WS_Client::intern_on_handshake(boost::system::error_code const& ec)
	{
		if (ec)
		{
			on_error(network_error(std::string("WS_Client::intern_on_handshake(): ")+ec.message()));
			return;
		}
		start();
		on_connect();
	}
	
	void WS_Client::connect(const std::string& _host, int port)
	{
		host = _host;
		boost::asio::ip::tcp::resolver rslv (ioc);
		boost::system::error_code err;
		auto const results = rslv.resolve(host,std::to_string(port),err);
		if (err)
		{
			on_error(network_error(std::string("WS_Client::connect() resolve-Fehler:")+err.message()));
			return;
		}
		boost::asio::async_connect(socket.next_layer(),results,boost::bind(&WS_Client::intern_on_connect,this,_1,_2));
	}
}