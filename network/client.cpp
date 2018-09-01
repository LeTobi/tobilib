#include "client.h"

namespace tobilib::stream
{
	WS_Client::WS_Client(boost::asio::io_context& _ioc): WS_Endpoint(_ioc), ioc(_ioc), rslv(_ioc)
	{
		on_close.notify([this](){
			active=false;
		},callback_position::early);
	};
	
	void WS_Client::intern_on_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::results_type results)
	{
		if (err)
		{
			on_error(network_error(std::string("WS_Client::connect() resolve-Fehler:")+err.message()));
			return;
		}
		boost::asio::async_connect(socket.next_layer(),results,boost::bind(&WS_Client::intern_on_connect,this,_1,_2));
	}
	
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
		if (active)
			on_error(network_error("Der Client ist bereits aktiv"));
		host = _host;
		boost::system::error_code err;
		rslv.async_resolve(host,std::to_string(port),boost::bind(&WS_Client::intern_on_resolve,this,_1,_2));
	}
}