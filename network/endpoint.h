#ifndef TC_NETWORK_ENDPOINT
#define TC_NETWORK_ENDPOINT

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "../general/exception.hpp"
#include "../general/timer.hpp"
#include <ctime>

namespace tobilib::stream
{
	class WS_Endpoint
	{
		friend class WS_Acceptor;

	// Generell //////////////////////////////////////////////////////////////////////////////////////////////////////
	public:
		WS_Endpoint ();

		enum class Status {Closed,Active,Idle,Shutdown};
		void tick();
		Status status() const;
		void reactivate(const std::string&);
		void shutdown();
		const boost::asio::ip::address& remote_ip() const;
		std::string mytrace() const;

		Warning_list warnings;
	protected:
		boost::asio::io_context ioc;
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket;
		
		void close_tcp();
	private:
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ioc);
		boost::asio::ip::address last_ip;
		Status _status = Status::Closed;

		void reactivate();

	// Schreiben //////////////////////////////////////////////////////////////////////////////////////////////////////
	private:
		std::string write_queue;
		std::string write_buffer;
		Timer write_close_timeout = Timer(5);
		enum class WriteStatus {Idle,Msg,Shutdown,Terminated,Error};
		WriteStatus _writestatus = WriteStatus::Idle;

		void write_begin();
		void write_end(const boost::system::error_code&, size_t);
		void write_close_begin();
		void write_close_end(const boost::system::error_code&);
		void write_reset();
		void write_tick();
		
	public:
		bool write_busy() const;
		void write(const std::string&);

	// Lesen //////////////////////////////////////////////////////////////////////////////////////////////////////
	private:
		boost::asio::streambuf read_buffer;
		std::string read_data;
		Timer read_warning_timer = Timer(10);
		Timer read_shutdown_timer = Timer(20);
		enum class ReadStatus {Idle,Reading,Error};
		ReadStatus _readstatus = ReadStatus::Idle;

		void read_begin();
		void read_end(const boost::system::error_code&, size_t);
		void read_reset();
		void read_tick();

	public:
		int read_size() const;
		std::string read(unsigned int len=0);
	};
}

#ifdef TC_AS_HPP
	#include "endpoint.cpp"
#endif

#endif