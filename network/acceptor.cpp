#include "acceptor.h"

namespace tobilib::stream
{
	void WS_Acceptor::intern_on_accept1(const boost::system::error_code& ec)
	{
		if (ec)
		{
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
			on_error(network_error(std::string("Fehler bei Handshake: ")+ec.message()));
			delete client;
			client = NULL;
			return;
		}
		WS_Endpoint* ep = client;
		client = NULL;
		ep->start();
		ep->on_close.notify([ep](){delete ep;},callback_position::late);
		on_accept(*ep);
	}
	
	void WS_Acceptor::next()
	{
		if (client!=NULL)
			return;
		client = new WS_Endpoint(ioc);
		accpt.async_accept(client->socket.next_layer(),boost::bind(&WS_Acceptor::intern_on_accept1,this,_1));
	}
	
	WS_Acceptor::~WS_Acceptor()
	{
		if (client!=NULL)
			delete client;
	}
}