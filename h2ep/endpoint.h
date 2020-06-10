#ifndef H2EP_ENDPOINT_H
#define H2EP_ENDPOINT_H

#include "../network/endpoint.h"
#include "event.h"
#include <map>

namespace tobilib::h2ep
{	
	template <class StrEndp>
	class Endpoint
	{
	public:
		typedef StrEndp Streamtype;
		typedef typename Streamtype::Status Status;

		Endpoint(StrEndp*, bool _responsable=false);
		~Endpoint();
		Endpoint(const Endpoint&) = delete;
		Endpoint& operator=(const Endpoint&) = delete;
		
		Status status() const;
		void reactivate(const Event&);
		bool write_busy() const;
		void send(const Event&);
		bool read_available() const;
		Event read();
		void shutdown();
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