#ifndef TC_ACCEPTOR_H
#define TC_ACCEPTOR_H

#include "endpoint.h"

namespace tobilib::stream
{	
	class WS_Acceptor
	{
	private:
		boost::asio::io_context ioc;
		boost::asio::ip::tcp::acceptor accpt;
		WS_Endpoint* client = new WS_Endpoint();
		int intern_port = 0;
		bool running = false;
		bool waiting = false;
		time_t connected = 0;

		void accept();
		void intern_on_accept1(const boost::system::error_code&);
		void intern_on_accept2(const boost::system::error_code&);
		void reset();
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ioc);

	public:
		typedef WS_Endpoint EndpointType;

		WS_Acceptor();
		WS_Acceptor(int _port);
		WS_Acceptor(const WS_Acceptor&) = delete;
		~WS_Acceptor();
		void operator=(const WS_Acceptor&) = delete;
		
		void tick();
		void open(int _port = 0);
		void close();
		bool opened() const;
		bool full() const;
		int port() const;
		WS_Endpoint* release();
		std::string mytrace() const;

		Warning_list warnings;
	};
}

#ifdef TC_AS_HPP
	#include "acceptor.cpp"
#endif

#endif