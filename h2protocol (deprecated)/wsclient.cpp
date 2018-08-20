#include "wsclient.h"

namespace tobilib
{
	void H2Client::connect(const StringPlus& host, int port)
	{
		try
		{
			ioc.reset();
			boost::asio::ip::tcp::resolver rslv (ioc);
			auto const results = rslv.resolve(host,std::to_string(port));
			boost::asio::connect(websock.next_layer(),results.begin(),results.end());
			websock.handshake(host.toString(),"/");
			read();
		}
		catch (boost::system::system_error& e)
		{
			throw h2client_error(std::string("Fehler beim Verbindungsaufbau: ")+e.what());
		}
	}
	
	void H2Client::disconnect()
	{
		try
		{
			websock.close(boost::beast::websocket::close_code::normal);
			// Diese werden von close übernommen:
			//websock.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			//websock.next_layer().close();
		}
		catch (boost::system::system_error& e)
		{
			throw h2client_error(std::string("Fehler beim beenden der Verbindung: ")+e.what());
		}
	}
	
	void H2Client::send(const H2Event& ev)
	{
		try
		{
			std::string msg = ev.stringify().toString();
			websock.write(boost::asio::buffer(msg.data(),msg.size()));
		}
		catch (boost::system::system_error& e)
		{
			throw h2client_error(std::string("Fehler beim Senden der Nachricht: ")+e.what());
		}
	}
	
	void H2Client::addEventListener(const StringPlus& evname, clientEventHandler handler)
	{
		handlers.emplace(evname,handler);
	}
	
	void H2Client::flush()
	{
		ioc.poll();
	}
	
	bool H2Client::connected() const
	{
		return websock.next_layer().is_open();
	}
	
	void H2Client::read()
	{
		websock.async_read(buffer, boost::bind(&H2Client::on_read,this,_1,_2));
	}
	
	void H2Client::on_read(const boost::system::error_code& ec, size_t len)
	{
		if (ec)
			throw h2client_error(std::string("Fehler beim Lesen der Nachricht: ")+ec.message());
		std::string msg;
		buffer.sgetn(&msg.front(),len);
		H2Event_parser parser;
		try {
			parser.feed(msg);
			while (parser.ready())
				call(parser.next());
		} catch (h2parser_error& err) {
			throw h2client_error(err.what());
		}
		read();
	}
	
	void H2Client::call (const H2Event& ev)
	{
		if (handlers.count(ev.name)<1)
		{
			if (fallback!=NULL)
				fallback(ev);
			return;
		}
		handlers.at(ev.name)(ev.data);
	}
}