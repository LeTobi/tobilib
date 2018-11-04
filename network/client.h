#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "endpoint.h"

namespace tobilib::stream
{
	class Client: public virtual Endpoint
	{
	public:
		Callback< > on_connect;
		virtual void connect(const std::string&, int) = 0;
		
		~Client(){};
	};
	
	class WS_Client: public virtual Client, public virtual WS_Endpoint
	{
	private:
		void intern_on_resolve(const boost::system::error_code&, boost::asio::ip::tcp::resolver::results_type);
		void intern_on_connect(const boost::system::error_code&, const boost::asio::ip::tcp::endpoint&);
		void intern_on_handshake(boost::system::error_code const&);
		
		boost::asio::ip::tcp::resolver rslv;
		std::string host;
		bool active = false;
		
	public:
		WS_Client(Process&);
		void connect(const std::string&, int);
	};
}

#ifdef TC_AS_HPP
	#include "client.cpp"
#endif

#endif