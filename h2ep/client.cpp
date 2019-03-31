#include "client.h"

namespace tobilib::h2ep
{
	template<class Streamclient>
	Client<Streamclient>::Client(): intern_client(), EndpointType(&intern_client)
	{ }
	
	template<class Streamclient>
	void Client<Streamclient>::connect(const std::string& host, int port)
	{
		intern_client.connect(host,port);
	}

	template<class Streamclient>
	void Client<Streamclient>::tick()
	{
		intern_client.tick();
		EndpointType::tick();
	}

	template<class Streamclient>
	typename Client<Streamclient>::Status Client<Streamclient>::status() const
	{
		return intern_client.status();
	}

	template<class Streamclient>
	std::string Client<Streamclient>::mytrace() const
	{
		std::string out = "h2ep::Client (";
		out += EndpointType::mytrace() +")";
		return out;
	}

	template class Client<stream::WS_Client>;
}