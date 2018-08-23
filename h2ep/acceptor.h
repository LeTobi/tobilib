#ifndef H2EP_ACCEPTOR_H
#define H2EP_ACCEPTOR_H

#include "../network/acceptor.h"
#include "endpoint.h"

namespace tobilib::h2ep
{
	template <class StrAccpt>
	class Acceptor
	{
	private:
		StrAccpt accpt;
		
		void intern_accept(stream::Endpoint*);
		
	public:
		Acceptor(boost::asio::io_context& ioc,int port);
		
		void next();
		Callback<Endpoint*> on_accept;
		Callback<const protocol_error&> on_error;
	};
	
	typedef Acceptor<stream::WS_Acceptor> WS_Acceptor;
}

#ifdef TC_AS_HPP
	#include "acceptor.cpp"
#endif

#endif