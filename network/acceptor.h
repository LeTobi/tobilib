#ifndef TC_ACCEPTOR_H
#define TC_ACCEPTOR_H

#include "endpoint.h"
#include <set>

namespace tobilib::stream
{	
	class WS_Acceptor
	{
	// DUMP //////////////////////////////////////////////////////////////////
	public:
		typedef WS_Endpoint EndpointType;
		
		WS_Acceptor();
		WS_Acceptor(int _port);
		WS_Acceptor(const WS_Acceptor&) = delete;
		~WS_Acceptor();
		void operator=(const WS_Acceptor&) = delete;

		WS_Endpoint* release();
		bool filled() const;
		int size() const;
		void tick();
		std::string mytrace() const;

		Warning_list warnings;

	private:
		std::set<WS_Endpoint*> available;
		
		boost::asio::io_context ioc;
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ioc);
		
	// LISTEN ////////////////////////////////////////////////////////////////
	public:
		enum class Status {Closed, Flush, Open};
		int intern_port;
		
		Status status() const;
		int port() const;
		void open(int _port = 0);
		void close();

	private:
		boost::asio::ip::tcp::acceptor accpt;
		WS_Endpoint* client = NULL;
		Status _status = Status::Closed;

		void accept();
		void on_accept(const boost::system::error_code&);

	// HANDSHAKE /////////////////////////////////////////////////////////////
	private:
		struct Connection
		{
			WS_Endpoint* endpoint;
			Timer timeout = Timer(10).set();
			enum class Status {Idle, Open, Cancel, Closed};
			Status _status = Status::Idle;
			boost::asio::ip::address remote_ip;

			Warning_list warnings;

			void tick();
			void on_handshake(const boost::system::error_code&);
			std::string mytrace() const;
		};
		std::set<Connection*> connections;
		int handshake_pendings() const;
	};
}

#ifdef TC_AS_HPP
	#include "acceptor.cpp"
#endif

#endif