#include "acceptor.h"

namespace tobilib::stream
{
	void WS_Acceptor::intern_on_accept1(const boost::system::error_code& ec)
	{
		if (ec)
		{
			if (on_error)
				on_error(network_error(std::string("Fehler bei Verbindungsaufbau: ")+ec.message()));
			delete client;
			client = NULL;
			return;
		}
		client->socket.async_accept(boost::bind(&WS_Acceptor::intern_on_accept2,this,_1));
	}
	
	void WS_Acceptor::intern_on_accept2(const boost::system::error_code& ec)
	{
		if (ec)
		{
			if (on_error)
				on_error(network_error(std::string("Fehler bei Handshake: ")+ec.message()));
			delete client;
			client = NULL;
			return;
		}
		client->start();
		if (on_accept)
		{
			Endpoint* p = client;
			client = NULL;
			on_accept(p);
		}
		else
		{
			delete client;
		}
	}
	
	void WS_Acceptor::next()
	{
		if (client!=NULL)
			return;
		client = new WS_Endpoint(ioc);
		accpt.async_accept(client->socket.next_layer(),boost::bind(&WS_Acceptor::intern_on_accept1,this,_1));
	}
}