#include "server.h"

namespace tobilib::stream
{
	
	template <class AccptType>
	Server<AccptType>::Server (boost::asio::io_context& _ioc, int port): accpt(_ioc,port)
	{
		accpt.on_accept = std::bind(&Server<AccptType>::on_accept,this,std::placeholders::_1);
		accpt.on_error = std::bind(&Server<AccptType>::accept_error,this,std::placeholders::_1);
	}
	
	template <class AccptType>
	void Server<AccptType>::on_accept(Endpoint* ep)
	{
		endpoints.push_back(ep);
		Endpoint_relay* epr = new Endpoint_relay(*ep);
		clients.push_back(epr);
		epr->master_on_receive = std::bind(&Server<AccptType>::_client_receive,this,epr);
		epr->master_on_error = std::bind(&Server<AccptType>::_client_error,this,epr,std::placeholders::_1);
		epr->master_on_close = std::bind(&Server<AccptType>::_client_stop,this,epr);
		if (on_connect)
			on_connect(epr);
		start();
	}
	
	template <class AccptType>
	void Server<AccptType>::accept_error(const network_error& err)
	{
		if (on_server_error)
			on_server_error(err);
	}
	
	template <class AccptType>
	void Server<AccptType>::_client_stop(Endpoint_relay* epr)
	{
		if (on_disconnect)
			on_disconnect(epr);
		if (epr->on_close)
			epr->on_close();
		for (int i=0;i<clients.size();i++)
		{
			if (clients[i]==epr)
			{
				delete clients[i];
				delete endpoints[i];
				endpoints.erase(endpoints.begin()+i);
				clients.erase(clients.begin()+i);
				if (_status == ServerState::shutdown && clients.size()==0)
					stop();
				return;
			}
		}
		throw network_error("Ein Client konnte nicht mehr identifiziert werden.");
	}
	
	template <class AccptType>
	void Server<AccptType>::_client_error(Endpoint_relay* epr, const network_error& err)
	{
		if (on_client_error)
			on_client_error(epr,err);
		if (epr->on_error)
			epr->on_error(err);
	}
	
	template <class AccptType>
	void Server<AccptType>::_client_receive(Endpoint_relay* epr)
	{
		if (on_receive)
			on_receive(epr);
		if (epr->on_receive)
			epr->on_receive();
	}
	
	template <class AccptType>
	void Server<AccptType>::start()
	{
		if (_status == ServerState::shutdown)
			return;
		_status = ServerState::running;
		accpt.next();
	}
	
	template <class AccptType>
	void Server<AccptType>::stop()
	{
		if (clients.size()==0)
		{
			_status = ServerState::down;
			if (on_stop)
				on_stop();
			return;
		}
		_status = ServerState::shutdown;
		for (auto& client: clients)
		{
			client->close();
		}
	}
	
	template class Server<WS_Acceptor>;
	//TODO
	//template class Server<TCP_Acceptor>
}