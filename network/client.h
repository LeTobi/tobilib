#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "endpoint.h"
#include "../general/timer.hpp"

namespace tobilib::stream
{
	class WS_Client: public WS_Endpoint
	{
	private:
		void intern_on_resolve(const boost::system::error_code&, boost::asio::ip::tcp::resolver::results_type);
		void intern_on_connect(const boost::system::error_code&, const boost::asio::ip::tcp::endpoint&);
		void intern_on_handshake(boost::system::error_code const&);
		
		boost::asio::ip::tcp::resolver rslv;
		std::string host;
		int port;
		bool _connecting = false;
		Timer timeout = Timer(10);
		
	public:
		enum class Status {Closed, Connecting, Active, Idle, Shutdown};
		typedef WS_Endpoint EndpointType;

		WS_Client();
		void connect(const std::string&, int);
		void tick();
		Status status() const;
		std::string mytrace() const;
	};
}

#ifdef TC_AS_HPP
	#include "client.cpp"
#endif

#endif