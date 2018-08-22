#ifndef H2EP_ENDPOINT_H
#define H2EP_ENDPOINT_H

#include "../network/endpoint.h"
#include "event.h"
#include "error.h"
#include <map>

namespace tobilib::h2ep
{	
	class Endpoint
	{
	public:
		Endpoint() {};
		Endpoint(stream::Endpoint*);
		
		typedef std::function<void(const JSObject&)> event_callback;
		
		Callback<const Event&> fallback;
		Callback<const protocol_error&> on_error;
		Callback< > on_close;
		
		void dock(stream::Endpoint*);
		bool connected() const;
		void send(const Event&);
		Callback_Ticket addEventListener(const std::string&, event_callback, callback_position pos = callback_position::early);
		void close();
		
	private:
		stream::Endpoint* stream = NULL;
		Event_parser parser;
		
		void intern_on_error(const network_error&);
		void intern_on_receive();
		void intern_on_close();
		void call(const Event&);
		std::map<std::string,Callback<const JSObject&>> callbacks;
	};
}

#ifdef TC_AS_HPP
	#include "endpoint.cpp"
#endif

#endif