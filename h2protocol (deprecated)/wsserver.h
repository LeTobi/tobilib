#ifndef WSServer_h
#define WSServer_h

#include "h2event.h"
#include "error.h"

// Netzwerk
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

// diverses
#include <string>
#include <vector>
#include <map>
#include <queue>

//#define WSS_DEBUG

namespace tobilib
{
	enum struct State {
			Setup,
			Idle,
			Shutdown,
			Closed
		};
	
	class WSServer;
	class WSSession;
	
	class WSClient
	{
	public:
		boost::property_tree::ptree data;
	
		WSClient(WSSession* wss);
		void call (H2Event);
		bool connected();
		boost::asio::ip::address ip;
		
	private:
		WSSession* session;
	};
	
	typedef void (*eventFunc) (JSObject, WSClient*);
	typedef void (*fallbackFunc) (H2Event, WSClient*);
	
	class WSSession
	{
	public:
		WSClient clientinfo;
		int index; // Index in der Liste des Servers, -1 wenn ausgeschlossen
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websock;
		State status = State::Setup;
		/*
			Setup: Warten auf Verbindung
			Idle: Event-Austausch
			Shutdown: Leeren des Schreibbuffers, saubere Trennung
			Closed: Warten auf alle callbacks
		*/
		
		WSSession(boost::asio::io_context&, WSServer*);
		void begin();
		void pushEvent(H2Event);
		void disconnect(); // Initialisiert eine serverseitige Beendigung der Verbindung (es folgt destroy() asynchron)
		void destroy(); // Beendet alle Prozesse und vernichtet diese Instanz
		
		#ifdef WSS_DEBUG
		static int nextid;
		#endif
		int debug_id = 0;
		
	private:
		WSServer* server;
		boost::asio::streambuf buffer;
		H2Event_parser parser;
		std::queue<H2Event> eventqueue;
		std::string transferdata;
		bool writing = false; // Beschreibt ob Schreibvorgang im Gang ist
		int pendingcallbacks = 0;
		
		void fireEvent();
		void parseError(const h2parser_error&);
		void read();
		void on_handshake(const boost::system::error_code&);
		void on_read(const boost::system::error_code&, size_t len);
		void on_write(const boost::system::error_code&, size_t len);
		void on_close(const boost::system::error_code&);
		
		
	};
	
	class WSServer
	{
	public:
		WSServer(short p):port(p) {};
		~WSServer();
		std::vector<WSClient*> clients;
		fallbackFunc fallback = NULL;
		
		void run();
		void stop(); // Fordert Unterbrechung aller Verbindungen (es folgt clear() asynchron)
		void clear(); // Unterbricht Prozesse und setzt server unmittelbar zurück
		// Vordefinierte Events:
		// intern_connect, intern_disconnect
		void addEventListener(std::string, eventFunc);
		void call(H2Event, WSClient*);
		
		void removeAt(int);
		
	private:
		std::vector<WSSession*> sessions;
		std::map<std::string, eventFunc> events;
		short port;
		boost::asio::io_context srv;
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workguard = boost::asio::make_work_guard(srv);
		boost::asio::ip::tcp::acceptor* accpt;
		State status = State::Setup;
		
		WSSession* addSession();
		void waitforclient();
		void acceptclient(WSSession*, const boost::system::error_code&);
	};
}

#ifdef TC_AS_HPP
	#include "wsserver.cpp"
#endif

#endif