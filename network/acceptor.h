#ifndef TC_ACCEPTOR_H
#define TC_ACCEPTOR_H

#include "endpoint.h"

namespace tobilib::stream
{
	class Acceptor
	{
	public:
		typedef std::function<void(Endpoint*)> callback;
		typedef std::function<void(const network_error&)> error_callback;
		
		virtual void next() = 0;
		callback on_accept;
		error_callback on_error;
	};
	
	class WS_Acceptor: public Acceptor
	{
	private:
		boost::asio::ip::tcp::acceptor accpt;
		boost::asio::io_context& ioc;
		WS_Endpoint* client = NULL;
		
		void intern_on_accept1(const boost::system::error_code&);
		void intern_on_accept2(const boost::system::error_code&);
		
	public:
		WS_Acceptor(boost::asio::io_context& _ioc, int port): ioc(_ioc), accpt(_ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)) {};
		
		void next();
	};
}

#ifdef TC_AS_HPP
	#include "acceptor.cpp"
#endif

#endif