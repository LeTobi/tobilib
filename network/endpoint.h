#ifndef TC_NETWORK_ENDPOINT
#define TC_NETWORK_ENDPOINT

#include "error.h"
#include "../general/callback.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace tobilib::stream
{
	enum class EndpointState {
		closed,
		shutdown,
		open
	};
	
	class Endpoint
	{
	protected:
		EndpointState _status = EndpointState::closed;
		
	public:
		Endpoint() {};
		
		Callback< > on_receive;
		Callback< > on_close;
		Callback<const network_error&> on_error;
		
		std::string received;
		
		EndpointState status() const { return _status; };
		bool connected() const {return _status == EndpointState::open; };
		
		virtual void start() = 0;
		virtual void write(const std::string&) = 0;
		virtual void close() = 0;
		virtual bool busy() const = 0;
		
		Endpoint(const Endpoint&) = delete;
		void operator=(const Endpoint&) = delete;
	};
	
	class WS_Endpoint: virtual public Endpoint
	{
	private:
		std::string outqueue;
		std::string writebuffer;
		boost::asio::streambuf buffer;
		bool writing = false;
		bool reading = false;
		bool closing = false;
		
		void intern_on_write(const boost::system::error_code&, size_t);
		void intern_on_read(const boost::system::error_code&, size_t);
		void intern_on_close(const boost::system::error_code&);
		void flush();
		void read();
		
	public:
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket;
		
		WS_Endpoint (boost::asio::io_context& _ioc): socket(_ioc) {};
		
		void start();
		void write(const std::string&);
		void close();
		bool busy() const;
	};
}

#ifdef TC_AS_HPP
	#include "endpoint.cpp"
#endif

#endif