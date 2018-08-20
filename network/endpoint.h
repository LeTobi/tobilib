#ifndef TC_NETWORK_ENDPOINT
#define TC_NETWORK_ENDPOINT

#include "error.h"
#include <functional>
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
		typedef std::function<void()> callback_type;
		
		callback_type on_receive;
		callback_type on_close;
		error_callback on_error;
		
		std::string received;
		
		EndpointState status() const { return _status; };
		bool connected() const {return _status == EndpointState::open; };
		
		virtual void start() = 0;
		virtual void write(const std::string&) = 0;
		virtual void close() = 0;
		
		virtual ~Endpoint() = 0;
	};
	
	class Endpoint_relay: public Endpoint
	{
	private:
		Endpoint& origin;
		
		void intern_receive();
		void intern_close();
		void intern_error(const network_error&);
		
	public:
		Endpoint_relay(Endpoint& o);
		
		callback_type master_on_receive;
		callback_type master_on_close;
		error_callback master_on_error;
		
		void start(){origin.start();};
		void write(const std::string& str) {origin.write(str);};
		void close() {origin.close();};
		void status_update() { _status = origin.status(); };
	};
	
	class WS_Endpoint: virtual public Endpoint
	{
	private:
		std::string outqueue;
		boost::asio::streambuf buffer;
		bool writing = false;
		bool reading = false;
		
		void intern_on_write(const boost::system::error_code&, size_t);
		void intern_on_read(const boost::system::error_code&, size_t);
		void intern_on_close(const boost::system::error_code&);
		void flush();
		void read();
		
	public:
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket;
		
		WS_Endpoint (boost::asio::io_context& _ioc): socket(_ioc) {};
		~WS_Endpoint() = default;
		
		void start();
		void write(const std::string&);
		void close();
	};
}

#ifdef TC_AS_HPP
	#include "endpoint.cpp"
#endif

#endif