#include "wsserver.h"
#include <sstream>
#include <boost/property_tree/json_parser.hpp>

#ifdef WSS_DEBUG
	#include <iostream>
#endif

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::property_tree;
using namespace boost::beast::websocket;

namespace tobilib
{
	inline void debug_log(string txt)
	{
		#ifdef WSS_DEBUG
		cout << txt << endl;
		#endif
	}

	WSClient::WSClient(WSSession* wss): session(wss)
	{
		debug_log("WSClient::WSClient()");
	}

	void WSClient::call(H2Event ev)
	{
		session->pushEvent(ev);
	}

	bool WSClient::connected()
	{
		return session->status == State::Idle;
	}

	#ifdef WSS_DEBUG
	int WSSession::nextid = 0;
	#endif

	WSSession::WSSession(boost::asio::io_context& ios, WSServer* s): websock(ios), server(s), clientinfo(this)
	{
		#ifdef WSS_DEBUG
		debug_id = nextid++;
		debug_log(string("!  initiate ")+to_string(debug_id));
		#endif
	}

	void WSSession::begin()
	{
		if (status != State::Setup)
			return;
		debug_log(string("!  begin() ")+to_string(debug_id));
		clientinfo.ip = websock.next_layer().remote_endpoint().address();
		writing = true;
		pendingcallbacks++;
		websock.async_accept(boost::bind(&WSSession::on_handshake,this,_1));
	}

	void WSSession::parseError(const h2parser_error& e)
	{
		debug_log(string("parseError() ")+to_string(debug_id));
		H2Event ev;
		ev.name = "format_error";
		ev.data.put("msg","Das H2 Event Protokoll wurde nicht eingehalten. Verbindung wird abgebrochen.");
		ev.data.put("error",e.what());
		ev.data.put("code",parser.buffer().toString());
		pushEvent(ev);
		disconnect();
	}

	void WSSession::pushEvent(H2Event ev)
	{
		eventqueue.push(ev);
		fireEvent();
	}

	void WSSession::fireEvent()
	{
		debug_log(string("fireEvent() ")+to_string(debug_id));
		if (status==State::Setup || writing)
			return;
		if (eventqueue.empty())
		{
			if (status==State::Shutdown)
				disconnect();
			return;
		}
		H2Event& ev = eventqueue.front();
		transferdata += ev.stringify().toString();
		pendingcallbacks++;
		websock.async_write(boost::asio::buffer(transferdata.data(),transferdata.size()),boost::bind(&WSSession::on_write,this,_1,_2));
		writing = true;
	}

	void WSSession::read()
	{
		debug_log(string("read() ")+to_string(debug_id));
		pendingcallbacks++;
		websock.async_read(buffer,boost::bind(&WSSession::on_read,this,_1,_2));
	}

	void WSSession::on_handshake(const boost::system::error_code& ec)
	{
		debug_log(string("on_handshake() ")+to_string(debug_id));
		writing = false;
		pendingcallbacks--;
		if (ec || status != State::Setup)
		{
			disconnect();
			return;
		}
		status = State::Idle;
		server->call(H2Event("intern_connect"),&clientinfo);
		read();
	}

	void WSSession::on_read(const boost::system::error_code& ec, size_t len)
	{
		debug_log(string("on_read() ")+to_string(debug_id));
		pendingcallbacks--;
		if (ec || status == State::Closed)
		{
			destroy();
			return;
		}
		if (status == State::Shutdown)
		{
			read();
			return;
		}
		string received (len,0);
		buffer.sgetn(&received.front(),len);
		parser.feed(received);
		try {
			while (parser.ready())
				server->call(parser.next(),&clientinfo);
		} catch (h2parser_error& e) {
			parseError(e);
		}
		read();
	}

	void WSSession::on_write(const boost::system::error_code& ec, size_t len)
	{
		debug_log(string("on_write() ")+to_string(debug_id));
		writing = false;
		pendingcallbacks--;
		if (status == State::Closed)
		{
			destroy();
			return;
		}
		if (ec)
		{
			// TODO: Hier müssten noch erneute Versuche gemacht werden.
			disconnect();
			return;
		}
		eventqueue.pop();
		fireEvent();
	}

	void WSSession::on_close(const boost::system::error_code& ec)
	{
		debug_log(string("on_close() ")+to_string(debug_id));
		pendingcallbacks--;
		// Laut Dokumentation müsste man warten, bis der Client die Verbindung abbricht.
		// In der Praxis ist das nicht der Fall.
		if (ec || true)
		{
			destroy();
		}
	}

	void WSSession::disconnect()
	{
		debug_log(string("disconnect() ")+to_string(debug_id));
		if (status==State::Closed)
		{
			destroy();
			return;
		}
		status = State::Shutdown;
		if (!writing)
		{
			debug_log("requesting disconnection of client...");
			pendingcallbacks++;
			websock.async_close(close_reason("Serverseitige Beendigung"),boost::bind(&WSSession::on_close,this,_1));
			return;
		}
		else
		{
			debug_log("Waiting for writing process to complete");
			return;
		}
		debug_log("disconnection ignored");
	}

	void WSSession::destroy()
	{
		debug_log(string("destroy() ")+to_string(debug_id));
		if (index!=-1)
		{
			int tmpi = index;
			index = -1;
			server->removeAt(tmpi);
		}
		if (status != State::Closed)
		{
			status = State::Closed;
			boost::system::error_code ec;
			pendingcallbacks++; // Falls callbacks innerhalb von shutdown() oder close() aufgerufen werden
			websock.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			websock.next_layer().close();
			server->call(H2Event("intern_disconnect"),&clientinfo);	
			pendingcallbacks--;
		}
		if (pendingcallbacks==0)
		{
			debug_log(string("!  deleting ")+to_string(debug_id));
			delete this;
			return;
		}
	}

	WSServer::~WSServer()
	{
		clear();
	}

	void WSServer::run()
	{
		debug_log("WSServer::run()");
		if (status != State::Setup)
			return;
		status = State::Idle;
		tcp::acceptor acptr (srv,tcp::endpoint(tcp::v4(),port));
		accpt = &acptr;
		waitforclient();
		debug_log("running io_context");
		srv.reset();
		debug_log("server started");
		srv.run();
	}

	void WSServer::stop()
	{
		debug_log("WSServer::stop()");
		status = State::Shutdown;
		for (int i=0;i<sessions.size();i++)
		{
			sessions[i]->disconnect();
		}
	}

	void WSServer::clear()
	{
		if (!(status==State::Idle || status==State::Shutdown))
			return;
		while (sessions.size()>0)
			removeAt(0);
		srv.stop();
		status = State::Setup;
	}

	void WSServer::addEventListener(string name, eventFunc fnc)
	{
		debug_log(string("Funktion hinzugefuegt: ")+name);
		events.emplace(name,fnc);
	}

	void WSServer::call(H2Event ev, WSClient* cli)
	{
		debug_log(StringPlus("WSServer::call(")+ev.name+")");
		if (events.count(ev.name)<1)
		{
			if (fallback != NULL)
				fallback(ev,cli);
			return;
		}
		events.at(ev.name)(ev.data,cli);
	}

	void WSServer::waitforclient()
	{
		debug_log("WSServer::waitforclient()");
		WSSession* sess = addSession();
		accpt->async_accept(sess->websock.next_layer(),boost::bind(&WSServer::acceptclient,this,sess,_1));
	}

	void WSServer::acceptclient(WSSession* sess, const boost::system::error_code& ec)
	{
		debug_log("WSServer::acceptclient()");
		if (status!=State::Idle)
			return;
		if (ec)
		{
			removeAt(sess->index);
			return;
		}
		sess->begin();
		waitforclient();
	}

	void WSServer::removeAt(int index)
	{
		debug_log(string("WSServer::removeAt() ")+to_string(index)+"/"+to_string(sessions.size()));
		// Diese Funktion soll von WSSession::destroy aufgerufen werden:
		if (sessions[index]->index!=-1)
		{
			sessions[index]->destroy();
			return;
		}
		sessions.erase(sessions.begin()+index);
		clients.erase(clients.begin()+index);
		for (int i=0;i<sessions.size();i++)
		{
			sessions[i]->index = i;
		}
		if (sessions.size()==0)
			clear();
	}

	WSSession* WSServer::addSession()
	{
		debug_log("WSServer::addSession()");
		WSSession* sess = new WSSession(srv,this);
		debug_log("session.push_back()");
		sessions.push_back(sess);
		sess->index = sessions.size()-1;
		debug_log("clients.push_back()");
		clients.push_back(&sess->clientinfo);
		return sess;
	}
}