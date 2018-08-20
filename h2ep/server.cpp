#include "server.h"

namespace tobilib::h2ep
{
	template<class AccptType>
	Server<AccptType>::Server(boost::asio::io_context& ioc, int port): server(ioc,port)
	{
		server.on_connect = std::bind(&Server<AccptType>::intern_on_connection,this,std::placeholders::_1);
		server.on_server_error = std::bind(&Server<AccptType>::intern_on_server_error,this,std::placeholders::_1);
		server.on_stop = std::bind(&Server<AccptType>::intern_on_stop,this);
	}
	
	template<class AccptType>
	void Server<AccptType>::call_event(Endpoint* ep, const Event& ev)
	{
		if (callbacks.count(ev.name)>0)
			callbacks.at(ev.name)(ep,ev.data);
		else if (fallback)
			fallback(ep,ev);
	}
	
	template<class AccptType>
	void Server<AccptType>::addEventListener(const std::string& name, client_event ev)
	{
		callbacks.emplace(name, ev);
	}
	
	template<class AccptType>
	void Server<AccptType>::start()
	{
		server.start();
	}
	
	template<class AccptType>
	void Server<AccptType>::stop()
	{
		server.stop();
	}
	
	template<class AccptType>
	void Server<AccptType>::intern_on_connection(stream::Endpoint* ep)
	{
		Endpoint* evep = new Endpoint();
		evep->on_close = std::bind(&Server<AccptType>::intern_on_disconnect,this,evep);
		evep->fallback = std::bind(&Server<AccptType>::call_event,this,evep,std::placeholders::_1);
		evep->on_error = std::bind(&Server<AccptType>::intern_on_client_error,this,evep,std::placeholders::_1);
		evep->dock(ep);
		clients.push_back(evep);
		if (on_connection)
			on_connection(evep);
	}
	
	template<class AccptType>
	void Server<AccptType>::intern_on_disconnect(Endpoint* ep)
	{
		if (on_disconnect)
			on_disconnect(ep);
		for (int i=0;i<clients.size();i++)
		{
			if (clients[i]==ep)
			{
				clients.erase(clients.begin()+i);
				delete ep;
				return;
			}
		}
		throw protocoll_error("Ein Client konnte nicht mehr identifiziert werden.");
	}
	
	template<class AccptType>
	void Server<AccptType>::intern_on_client_error(Endpoint* ep,const protocoll_error& err)
	{
		if (on_client_error)
			on_client_error(ep,err);
	}
	
	template<class AccptType>
	void Server<AccptType>::intern_on_server_error(const network_error& err)
	{
		if (on_server_error)
			on_server_error(protocoll_error(err.what()));
	}
	
	template<class AccptType>
	void Server<AccptType>::intern_on_stop()
	{
		if (on_stop)
			on_stop();
	}
	
	template class Server<stream::WS_Acceptor>;
	//TODO
	//template class Server<TCP_Acceptor>
}