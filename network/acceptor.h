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
		Callback<Endpoint&> on_accept;
		Callback<const network_error&> on_error;
	};
	
	class WS_Acceptor: public Acceptor
	{
	private:
		boost::asio::ip::tcp::acceptor accpt;
		boost::asio::io_context& ioc;
		WS_Endpoint* client = NULL;
		
		void intern_on_accept1(const boost::system::error_code&);
		void intern_on_accept2(const boost::system::error_code&);
		void intern_cleanup(WS_Endpoint*);
		
	public:
		WS_Acceptor(boost::asio::io_context& _ioc, int port): ioc(_ioc), accpt(_ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)) {};
		~WS_Acceptor();
		
		void next();
	};
}

#ifdef TC_AS_HPP
	#include "acceptor.cpp"
#endif

#endif