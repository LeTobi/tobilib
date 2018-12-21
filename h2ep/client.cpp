#include "client.h"

namespace tobilib::h2ep
{
	template<class Streamclient>
	Client<Streamclient>::Client(): intern_client(), EndpointType(&intern_client)
	{ }
	
	template<class Streamclient>
	std::string Client<Streamclient>::mytrace() const
	{
		std::string out = "h2ep::Client ";
		if (connecting())
			out+="connecting";
		else if (EndpointType::status()==EndpointStatus::closed)
			out+="disconnected";
		else
			out+="connected";
		out += " ";
		out += EndpointType::mytrace();
		return out;
	}

	template<class Streamclient>
	void Client<Streamclient>::connect(const std::string& host, int port)
	{
		intern_client.connect(host,port);
	}
	
	template<class Streamclient>
	bool Client<Streamclient>::connecting() const
	{
		return intern_client.connecting();
	}
	
	template class Client<stream::WS_Client>;
}