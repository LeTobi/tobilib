#ifndef webcapserver_h
#define webcapserver_h

#include <linux/videodev2.h>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include "stringplus.h"
#include <vector>
#include <atomic>
#include <mutex>
#include <iostream>

namespace webcap
{
	enum ptr_type
	{
		memory,
		map,
	};
	
	class Buffer
	{
	public:
		char* start() {return _start;}
		int length() {return _length;}
		int used = 0;
		
		void allocate(int);
		void map(int, int, int);
		void clear();
		~Buffer();
		
	private:
		ptr_type type;
		char* _start = NULL;
		int _length = 0;
	};
	
	class cam_error: public std::exception
	{
	public:
		std::string info;
		
		cam_error(std::string txt): info(txt) {}
		cam_error(const char* txt): info(txt) {}
		const char* what() const noexcept {return info.c_str();}
	};
	
	class Capture
	{
	public:
		void open(std::string p = "");
		std::string check();
		void streamon();
		void read();
		void streamoff();
		void close();
		
		Buffer message_data;
		
	private:
		Buffer cam_data;
		Buffer raw_data;
		int imgwidth;
		int imgheight;
		v4l2_buffer bufferinfo;
		
		int device = -1;
		bool streaming = false;
		bool check_format(const v4l2_format&);
		std::string path;
	};
	
	class Session
	{
	public:
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket;
		boost::asio::ip::address ip;
		bool alive = true;
		
		Session (boost::asio::io_service& iosrv):socket(iosrv){}
		
		void start();
		void transmit(Buffer&);
		
		void close(); // kein delete!
	};
	
	class Streamserver
	{
	public:
		Streamserver (int p): ioc(), acceptor(ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),p)) {}
		~Streamserver();
		
		void run(std::string);
		void stop();
		void allow_ip(boost::asio::ip::address);
		void reject_ip(boost::asio::ip::address);
		
		bool active();
		int client_max(int m=0);
		int client_count();
		void logstream(std::ostream*);
		
	private:
		bool ready = true;
		std::atomic<bool> running {true};
		std::vector<Session*> sessions;
		std::atomic<int> maxsessions {1};
		boost::asio::io_context ioc;
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workguard = boost::asio::make_work_guard(ioc);
		boost::asio::ip::tcp::acceptor acceptor;
		std::mutex loglock;
		std::ostream* log = &std::cout;
		std::mutex listlock;
		std::vector<boost::asio::ip::address> whitelist;
		std::thread mainthread;
		Capture cam;
		Session* idle_session = NULL;
		
		
		// main functions
		void main();
		void loop();
		void idle();
		
		// side functions
		void client_remove(Session*);
		void async_listen();
		void on_connection(const boost::system::error_code&);
		bool is_allowed(boost::asio::ip::address);
		void writelog(const char *);
		void writelog(std::string);
	};
}

#endif