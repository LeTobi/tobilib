#ifndef H2EP_ENDPOINT_H
#define H2EP_ENDPOINT_H

#include "../network/endpoint.h"
#include "event.h"
#include <map>

namespace tobilib::h2ep
{	
	typedef stream::EndpointStatus EndpointStatus;

	template <class StrEndp>
	class Endpoint
	{
	public:
		typedef StrEndp Streamtype;

		Endpoint(StrEndp*, bool _responsable=false);
		~Endpoint();
		Endpoint(const Endpoint&) = delete;
		Endpoint& operator=(const Endpoint&) = delete;
		
		EndpointStatus status() const;
		bool inactive() const;
		void inactive_checked();
		bool busy() const;
		void send(const Event&);
		bool readable() const;
		Event read();
		void close();
		void tick();
		const boost::asio::ip::address& remote_ip() const;
		std::string mytrace() const;

		Warning_list warnings;

	private:
		bool responsable;
		StrEndp* stream = NULL;
		Event_parser parser;		
	};

	typedef Endpoint<stream::WS_Endpoint> WS_Endpoint;
}

#ifdef TC_AS_HPP
	#include "endpoint.cpp"
#endif

#endif