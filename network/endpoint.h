#ifndef TC_NETWORK_ENDPOINT
#define TC_NETWORK_ENDPOINT

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "../general/exception.hpp"

namespace tobilib::stream
{
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
	
	class WS_Endpoint
	{
		friend class WS_Acceptor;

	protected:
		boost::asio::io_context ioc;
		boost::beast::websocket::stream<boost::asio::ip::tcp::socket> socket;
		
		/** Signalisiert wenn die Verbindung hergestellt wurde **/
		void start();

		/** Schliesst die Verbindung direkt und dreckig **/
		void close_socket();

	private:
	// Generell
		/** Zustand von WS_Endpoint **/
		enum Flags {
			writing   = 0b000000001, /** asynchrones schreiben aktiv **/
			reading   = 0b000000010, /** asynchrones lesen aktiv **/
			closing   = 0b000000100, /** asynchrones schliessen aktiv **/
			timing    = 0b000001000, /** asynchrones timing aktiv **/
			breakdown = 0b000010000, /** fordert unterbrechung der Verbindung **/
			ready     = 0b000100000, /** Bereit für start **/
			idle      = 0b001000000, /** Beschreibt, ob gerade nichts getan wird **/
			warned    = 0b010000000, /** Beschreibt, ob eine timeoutwarnung aussteht **/
		};

		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard = boost::asio::make_work_guard(ioc);
		EndpointStatus _status = EndpointStatus::closed;
		unsigned int _state = 0;
		boost::asio::ip::address last_ip;

	// Schreiben
		std::string outqueue;
		std::string writebuffer;

		void flush();
		void send_close();
		void intern_on_write(const boost::system::error_code&, size_t);
		void intern_on_close(const boost::system::error_code&);

	// Lesen
		boost::asio::streambuf buffer;
		std::string received;

		void intern_read();
		void intern_on_read(const boost::system::error_code&, size_t);

	// Zeitmanagement
		boost::asio::system_timer timer;

		void timerset();
		void intern_on_timeout(const boost::system::error_code&);

	public:
		WS_Endpoint ();
		
		void tick();
		bool readable(unsigned int len=1) const;
		std::string read(unsigned int len=0);
		bool busy() const;
		void write(const std::string&);
		bool inactive();
		void close();
		const boost::asio::ip::address& remote_ip() const;
		std::string mytrace() const;
		

		EndpointStatus status() const { return _status; };
		Warning_list warnings;
	};
}

#ifdef TC_AS_HPP
	#include "endpoint.cpp"
#endif

#endif