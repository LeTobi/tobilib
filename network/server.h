#ifndef TC_SERVER_H
#define TC_SERVER_H

#include "endpoint.h"
#include "acceptor.h"
#include <vector>

namespace tobilib::stream
{	
	enum class ServerState
	{
		running,
		shutdown,
		down
	};
	
	template<class AccptType>
	class Server
	{
	private:
		AccptType accpt;
		std::vector<Endpoint*> endpoints;
		
		void on_accept(Endpoint*);
		void accept_error(const network_error&);
		void _client_stop(Endpoint_relay*);
		void _client_error(Endpoint_relay*, const network_error& err);
		void _client_receive(Endpoint_relay*);
		
	public:
		typedef std::function<void(Endpoint*)> client_event;
		typedef std::function<void(Endpoint*,const network_error&)> client_error;
		typedef std::function<void()> server_event;
		typedef std::function<void(const network_error&)> server_error;
		
		std::vector<Endpoint*> clients;
		ServerState _status = ServerState::down;
		
		client_event on_receive;
		client_event on_connect;
		client_event on_disconnect;
		client_error on_client_error;
		server_error on_server_error;
		server_event on_stop;
		
		Server (boost::asio::io_context&, int);
		void start();
		void stop();
	};
	
	typedef Server<WS_Acceptor> WS_Server;
	// todo: typedef Server<TCP_Acceptor> TCP_Server;
}

#ifdef TC_AS_HPP
	#include "server.cpp"
#endif

#endif