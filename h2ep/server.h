#ifndef H2EP_SERVER_H
#define H2EP_SERVER_H

#include "../network/server.h"
#include "endpoint.h"

namespace tobilib::h2ep
{
	template<class AccptType>
	class Server
	{
	public:
		typedef std::function<void(Endpoint*)> client_callback;
		typedef std::function<void(Endpoint*,const Event&)> client_fallback;
		typedef std::function<void(Endpoint*,const JSObject&)> client_event;
		typedef std::function<void(Endpoint*,const protocoll_error&)> client_error;
		typedef std::function<void()> server_callback;
		typedef std::function<void(const protocoll_error&)> server_error;
		
		Server(boost::asio::io_context&, int);
		
		client_fallback fallback;
		client_callback on_connection;
		client_callback on_disconnect;
		client_error on_client_error;
		server_error on_server_error;
		server_callback on_stop;
		
		void start();
		void stop();
		void addEventListener(const std::string&, client_event);
		std::vector<Endpoint*> clients;
		
	private:
		stream::Server<AccptType> server;
		std::map<std::string, client_event> callbacks;
		
		void call_event(Endpoint*, const Event&);
		void intern_on_connection(stream::Endpoint*);
		void intern_on_disconnect(Endpoint*);
		void intern_on_client_error(Endpoint*,const protocoll_error&);
		void intern_on_server_error(const network_error&);
		void intern_on_stop();
	};
	
	typedef Server<stream::WS_Acceptor> WS_Server;
}

#ifdef TC_AS_HPP
	#include "server.cpp"
#endif

#endif