#include "acceptor.h"

namespace tobilib::stream
{
	WS_Acceptor::WS_Acceptor(): accpt(ioc)
	{ }

	WS_Acceptor::WS_Acceptor(int _port): intern_port(_port), accpt(ioc)
	{ }

	WS_Acceptor::~WS_Acceptor()
	{
		if (client!=NULL)
			delete client;
		while (connections.size()>0)
		{
			delete (*connections.begin())->endpoint;
			delete *connections.begin();
			connections.erase(connections.begin());
		}
		while (available.size()>0)
		{
			delete *available.begin();
			available.erase(available.begin());
		}
	}

	WS_Endpoint* WS_Acceptor::release()
	{
		if (available.size()==0)
		{
			Exception e ("Es stehen keine Verbindungen bereit");
			e.trace.push_back("release()");
			e.trace.push_back(mytrace());
			throw e;
		}
		WS_Endpoint* out = *available.begin();
		available.erase(out);
		return out;
	}

	bool WS_Acceptor::filled() const
	{
		return available.size()>0;
	}

	int WS_Acceptor::size() const
	{
		return available.size() + handshake_pendings();
	}

	void WS_Acceptor::tick()
	{
		ioc.poll_one();
		auto it = connections.begin();
		while (it != connections.end())
		{
			auto conn = *it;
			it++;

			conn->tick();
			warnings.overtake(conn->warnings,mytrace());
			if (conn->_status == Connection::Status::Open)
			{
				available.insert(conn->endpoint);
				connections.erase(conn);
			}
			else if (conn->_status == Connection::Status::Closed)
			{
				connections.erase(conn);
			}
		}
	}

	std::string WS_Acceptor::mytrace() const
	{
		return std::string("WS_Acceptor Port ") + std::to_string(intern_port);
	}

	WS_Acceptor::Status WS_Acceptor::status() const
	{
		return _status;
	}

	int WS_Acceptor::port() const
	{
		return intern_port;
	}

	void WS_Acceptor::open(int _port)
	{
		if (_status == Status::Open)
		{
			Exception e ("Acceptor ist bereits aktiv");
			e.trace.push_back(std::string("WS_Acceptor::open(")+std::to_string(_port)+")");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			return;
		}
		if (_port>0)
			intern_port = _port;
		_status = Status::Open;
		boost::asio::ip::tcp::endpoint ep (boost::asio::ip::tcp::v4(), intern_port);
		accpt.open(ep.protocol());
		accpt.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		accpt.bind(ep);
		accpt.listen();
		accept();
	}

	void WS_Acceptor::close()
	{
		if (_status != Status::Open)
			return;
		_status = Status::Flush;
		accpt.close();
	}

	void WS_Acceptor::accept()
	{
		if (client!=NULL)
		{
			Exception e ("Fehler bei Internem Ablauf");
			e.trace.push_back("accept()");
			e.trace.push_back(mytrace());
			throw e;
		}
		client = new WS_Endpoint();
		accpt.async_accept(client->socket.next_layer(),boost::bind(&WS_Acceptor::on_accept,this,_1));
	}

	void WS_Acceptor::on_accept(const boost::system::error_code& ec)
	{
		if (ec)
		{
			Exception e ("Fehler beim Verbindungsaufbau: ");
			e+=ec.message();
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			delete client;
			client = NULL;
		}
		else
		{
			Connection* conn = new Connection();
			conn->endpoint = client;
			try {
				conn->remote_ip = client->socket.next_layer().remote_endpoint().address();
			} catch (std::exception& err) {
				Exception e (err.what());
				e.trace.push_back("on_accept() - IP auslesen");
				e.trace.push_back(mytrace());
				warnings.push_back(e);
			}
			connections.insert(conn);
			client->socket.async_accept(boost::bind(&WS_Acceptor::Connection::on_handshake,conn,_1));
			client = NULL;
		}
		if (_status == Status::Open)
			accept();
		else if (_status == Status::Flush)
			_status = Status::Closed;
	}
	
	void WS_Acceptor::Connection::tick()
	{
		endpoint->tick();
		warnings.overtake(endpoint->warnings,mytrace());
		if (timeout.due())
		{
			Exception e ("Timeout bei Handshake");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			endpoint->close_tcp();
			_status == Status::Cancel;
		}
	}

	void WS_Acceptor::Connection::on_handshake(const boost::system::error_code& ec)
	{
		timeout.disable();
		if (ec)
		{
			if (_status == Status::Idle)
			{
				Exception e ("Fehler bei Handshake: ");
				e+=ec.message();
				e.trace.push_back("on_handshake()");
				e.trace.push_back(mytrace());
				warnings.push_back(e);
				endpoint->close_tcp();
			}
			_status = Status::Closed;
			return;
		}
		_status = Status::Open;
	}

	std::string WS_Acceptor::Connection::mytrace() const
	{
		std::string stat;
		switch (_status)
		{
			case Status::Idle:
				stat = "Idle";
				break;
			case Status::Closed:
				stat = "Closed";
				break;
			case Status::Open:
				stat = "Open";
				break;
		}
		return std::string("WS_Acceptor::Connection ")+ stat + " remote-address: " + remote_ip.to_string();
	}

	int WS_Acceptor::handshake_pendings() const
	{
		int count = 0;
		for (auto& conn: connections)
		{
			if (conn->_status == Connection::Status::Open || conn->_status == Connection::Status::Idle)
				count++;
		}
		return count;
	}

}