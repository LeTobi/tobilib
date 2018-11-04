#ifndef TC_NETWORK_ENDPOINT
#define TC_NETWORK_ENDPOINT

#include "../general/callback.hpp"
#include "../thread/process.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <exception>

namespace tobilib::stream
{
	class network_error: public std::exception
	{
	public:
		template<class strT> network_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};

	/** Beschreibt den Status eines Endpunkts. Bestimmt das interne Verhalten */
	enum class EndpointStatus {
		/** Lesen und Schreiben aktiv */
		open,

		/** Schliessen wird vorbereitet: Schreibbuffer wird gelehrt */
		shutdown,

		/** Warten auf Antwort der Gegenseite */
		closing,

		/** Endpunkt geschlossen (on_close bereits ausgeführt) */
		closed
	};

	/* Informationen zum Zustand des Endpoints */
	enum EndpointFlags {
		writing = 0b000001, /** asynchrones schreiben aktiv **/
		reading = 0b000010, /** asynchrones lesen aktiv **/
		closing = 0b000100, /** asynchrones schliessen aktiv **/
		timing  = 0b001000, /** asynchrones timing aktiv **/
		breakdown = 0b010000, /** fordert unterbrechung der Verbindung **/
		ready   = 0b100000, /** Bereit für start **/
		idle =  0b01000000, /** Beschreibt, ob gerade nichts getan wird **/
		warned= 0b10000000 /** Beschreibt, ob eine timeoutwarnung aussteht **/
	};
	
	class Endpoint
	{
	protected:
		EndpointStatus _status = EndpointStatus::closed;
		unsigned int _state = 0;

	public:
		Endpoint() {};
		
		Callback< > on_receive;
		Callback< > on_close;
		Callback<const network_error&> on_error;
		Callback<bool> on_timeout;
		
		std::string received;
		
		EndpointStatus status() const { return _status; };
		bool connected() const {return _status == EndpointStatus::open; };
		
		virtual void start() = 0;
		virtual void write(const std::string&) = 0;
		virtual void close() = 0;
		virtual bool busy() const = 0;
		
		Endpoint(const Endpoint&) = delete;
		void operator=(const Endpoint&) = delete;
	};
	
	class WS_Endpoint: virtual public Endpoint
	{
		friend class WS_Acceptor;

	protected:
		Process myprocess;
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket;

	private:
		std::string outqueue;
		std::string writebuffer;
		boost::asio::streambuf buffer;
		boost::asio::system_timer timer;

		void intern_on_write(const boost::system::error_code&, size_t);
		void intern_on_read(const boost::system::error_code&, size_t);
		void intern_on_close(const boost::system::error_code&);
		void intern_on_timeout(const boost::system::error_code&);

		/** Diese Funktion ist zuständig für die Anpassung des Status. NUR diese Funktion!
		*/
		void tick();

		void flush();
		void read();
		void timerset();
		void send_close();

	public:
		WS_Endpoint (Process&);
		
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