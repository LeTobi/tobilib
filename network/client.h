#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "endpoint.h"

namespace tobilib::stream
{
	class Client: public virtual Endpoint
	{
	public:
		callback_type on_connect;
		virtual void connect(const std::string&, int) = 0;
		
		~Client(){};
	};
	
	class WS_Client: public virtual Client, public virtual WS_Endpoint
	{
	private:
		void intern_on_connect(const boost::system::error_code&, const boost::asio::ip::tcp::endpoint&);
		void intern_on_handshake(boost::system::error_code const&);
		
		boost::asio::io_context& ioc;
		std::string host;
		
	public:
		WS_Client(boost::asio::io_context& _ioc): WS_Endpoint(_ioc), ioc(_ioc) {};
		void connect(const std::string&, int);
		~WS_Client(){};
	};
}

#ifdef TC_AS_HPP
	#include "client.cpp"
#endif

#endif