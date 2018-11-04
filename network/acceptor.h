#ifndef TC_ACCEPTOR_H
#define TC_ACCEPTOR_H

#include "endpoint.h"

namespace tobilib::stream
{
	class Acceptor
	{
	public:
		Acceptor() {};
		Acceptor(const Acceptor&) = delete;
		void operator=(const Acceptor&) = delete;
		
		virtual void next() = 0;
		Callback<Endpoint*> on_accept;
		Callback<const network_error&> on_error;
	};
	
	class WS_Acceptor: public Acceptor
	{
	private:
		boost::asio::ip::tcp::acceptor accpt;
		Process& parentproc;
		Process myprocess;
		WS_Endpoint* client = NULL;
		
		void intern_on_accept1(const boost::system::error_code&);
		void intern_on_accept2(const boost::system::error_code&);
		
	public:
		WS_Acceptor(Process& proc, int port): parentproc(proc), myprocess(proc), accpt(myprocess,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)) {};
		~WS_Acceptor();
		
		void next();
	};
}

#ifdef TC_AS_HPP
	#include "acceptor.cpp"
#endif

#endif