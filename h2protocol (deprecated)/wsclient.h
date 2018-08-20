#ifndef WSCLIENT_H
#define WSCLIENT_H

#include "h2event.h"
#include "error.h"
#include <map>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace tobilib
{
	typedef void (*clientEventHandler)(JSObject);
	typedef void (*clientEventFallback)(H2Event);
	
	class H2Client
	{
	private:
		std::map<StringPlus, clientEventHandler> handlers;
		boost::asio::io_context ioc;
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workguard = boost::asio::make_work_guard(ioc);
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websock;
		boost::asio::streambuf buffer;
		
		void read();
		void on_read(const boost::system::error_code&, size_t);
		void call(const H2Event&);
		
	public:
		H2Client(): ioc(), websock(ioc) {};
		
		clientEventFallback fallback = NULL;
	
		void connect(const StringPlus&, int);
		void disconnect();
		void send(const H2Event& ev);
		// reservierte events: intern_connect / intern_disconnect / format_error
		void addEventListener(const StringPlus&, clientEventHandler);
		void flush();
		bool connected() const;
	};
}

#ifdef TC_AS_HPP
	#include "wsclient.cpp"
#endif

#endif