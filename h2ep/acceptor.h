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

	public:
		typedef Endpoint<typename StrAccpt::EndpointType> EndpointType;

		Acceptor();
		Acceptor(int _port);

		void tick();
		void open(int _port=0);
		void close();
		bool opened() const;
		bool full() const;
		int port() const;
		EndpointType* release();
		std::string mytrace() const;

		Warning_list warnings;
	};
	
	typedef Acceptor<stream::WS_Acceptor> WS_Acceptor;
}

#ifdef TC_AS_HPP
	#include "acceptor.cpp"
#endif

#endif