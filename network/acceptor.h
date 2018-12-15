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
		
		virtual void start() = 0;
		virtual void stop() = 0;
		Callback<Endpoint*> on_accept;
		Callback<const network_error&> on_error;
	};
	
	class WS_Acceptor: public Acceptor
	{
	private:
		Process& parentproc;
		Process myprocess;
		boost::asio::ip::tcp::acceptor accpt;
		WS_Endpoint* client = NULL;
		
		void intern_on_accept1(const boost::system::error_code&);
		void intern_on_accept2(const boost::system::error_code&);
		
	public:
		WS_Acceptor(Process& proc, int port): parentproc(proc), myprocess(proc), accpt(myprocess,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)) {};
		~WS_Acceptor();
		
		void start();
		void stop();
	};
}

#ifdef TC_AS_HPP
	#include "acceptor.cpp"
#endif

#endif