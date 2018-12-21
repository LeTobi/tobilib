#ifndef H2EP_CLIENT_H
#define H2EP_CLIENT_H

#include "endpoint.h"
#include "../network/client.h"

namespace tobilib::h2ep
{
	template<class Streamclient>
	class Client: public Endpoint<typename Streamclient::EndpointType>
	{
	private:
		Streamclient intern_client;

	public:
		typedef Streamclient ClientType;
		typedef Endpoint<typename Streamclient::EndpointType> EndpointType;

		Client();
		
		void connect(const std::string&, int);
		bool connecting() const;
		std::string mytrace() const;
	};
	
	typedef Client<stream::WS_Client> WS_Client;
}

#ifdef TC_AS_HPP
	#include "client.cpp"
#endif

#endif