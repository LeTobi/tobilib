#ifndef H2EP_CLIENT_H
#define H2EP_CLIENT_H

#include "endpoint.h"
#include "../network/client.h"

namespace tobilib::h2ep
{
	template<class Streamclient>
	class Client: public Endpoint
	{
	private:
		Streamclient intern_client;
		
		void intern_on_connect();
		
	public:
		Client(Process&);
		~Client(){};
		
		void connect(const std::string&, int);
		
		Callback< > on_connect;
	};
	
	typedef Client<stream::WS_Client> WS_Client;
}

#ifdef TC_AS_HPP
	#include "client.cpp"
#endif

#endif