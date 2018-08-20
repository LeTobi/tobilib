#include "client.h"

namespace tobilib::h2ep
{
	template<class Streamclient>
	Client<Streamclient>::Client(boost::asio::io_context& ioc): intern_client(ioc)
	{
		intern_client.on_connect = std::bind(&Client::intern_on_connect,this);
		dock(&intern_client);
	}
	
	template<class Streamclient>
	void Client<Streamclient>::connect(const std::string& host, int port)
	{
		intern_client.connect(host,port);
	}
	
	template<class Streamclient>
	void Client<Streamclient>::intern_on_connect()
	{
		if (on_connect)
			on_connect();
	}
	
	template class Client<stream::WS_Client>;
	// TODO
	// template class Client<stream::TCP_Client>;
}