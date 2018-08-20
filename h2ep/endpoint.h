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
		typedef std::function<void()> empty_callback;
		typedef std::function<void(const Event&)> default_callback;
		typedef std::function<void(const JSObject&)> event_callback;
		typedef std::function<void(const protocoll_error&)> error_callback;
		
		default_callback fallback;
		error_callback on_error;
		empty_callback on_close;
		
		void dock(stream::Endpoint*);
		void send(const Event&);
		void addEventListener(const std::string&, event_callback);
		void close();
		
	private:
		stream::Endpoint* stream = NULL;
		Event_parser parser;
		
		void intern_on_error(const network_error&);
		void intern_on_receive();
		void intern_on_close();
		void call(const Event&);
		std::map<std::string,event_callback> callbacks;
	};
}

#ifdef TC_AS_HPP
	#include "endpoint.cpp"
#endif

#endif