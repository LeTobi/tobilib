#include "acceptor.h"

namespace tobilib::stream
{
	void WS_Acceptor::intern_on_accept1(const boost::system::error_code& ec)
	{
		if (ec)
		{
			delete client;
			client = NULL;
			on_error(network_error(std::string("Fehler bei Verbindungsaufbau: ")+ec.message()));
			return;
		}
		client->socket.async_accept(boost::bind(&WS_Acceptor::intern_on_accept2,this,_1));
	}
	
	void WS_Acceptor::intern_on_accept2(const boost::system::error_code& ec)
	{
		if (ec)
		{
			delete client;
			client = NULL;
			on_error(network_error(std::string("Fehler bei Handshake: ")+ec.message()));
			return;
		}
		WS_Endpoint* ep = client;
		client = NULL;
		ep->start();
		start();
		on_accept(ep);
	}
	
	void WS_Acceptor::start()
	{
		if (client!=NULL)
			return;
		client = new WS_Endpoint(parentproc);
		accpt.async_accept(client->socket.next_layer(),boost::bind(&WS_Acceptor::intern_on_accept1,this,_1));
	}
	
	void WS_Acceptor::stop()
	{
		myprocess.restart();
		if (client!=NULL)
			delete client;
	}

	WS_Acceptor::~WS_Acceptor()
	{
		stop();
	}
}