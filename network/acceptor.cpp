#include "acceptor.h"

namespace tobilib::stream
{
	WS_Acceptor::WS_Acceptor(): accpt(ioc)
	{ }

	WS_Acceptor::WS_Acceptor(int _port): intern_port(_port), accpt(ioc)
	{ }

	void WS_Acceptor::accept()
	{
		if (!running)
		{
			Exception e ("Interner Fehler: WS_Acceptor::accept() in angehaltenem Zustand");
			e.trace.push_back(mytrace());
			throw e;
		}
		if (waiting || connected>0)
			return;
		waiting = true;
		accpt.async_accept(client->socket.next_layer(),boost::bind(&WS_Acceptor::intern_on_accept1,this,_1));
	}

	void WS_Acceptor::intern_on_accept1(const boost::system::error_code& ec)
	{
		waiting = false;
		if (ec)
		{
			Exception e ("Fehler beim Verbindungsaufbau: ");
			e+=ec.message();
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			reset();
			return;
		}
		time(&connected);
		client->socket.async_accept(boost::bind(&WS_Acceptor::intern_on_accept2,this,_1));
	}
	
	void WS_Acceptor::intern_on_accept2(const boost::system::error_code& ec)
	{
		connected = 0;
		if (ec)
		{
			Exception e ("Fehler bei Handshake: ");
			e+=ec.message();
			e.trace.push_back(mytrace());
			reset();
			return;
		}
		client->start();
	}
	
	void WS_Acceptor::reset()
	{
		delete client;
		client = new WS_Endpoint();
	}

	std::string WS_Acceptor::mytrace() const
	{
		return std::string("WS_Acceptor Port ") + std::to_string(intern_port);
	}

	void WS_Acceptor::open(int _port)
	{
		if (running)
		{
			Exception e ("Acceptor ist bereits aktiv");
			e.trace.push_back(std::string("WS_Acceptor::open(")+std::to_string(_port)+")");
			e.trace.push_back(mytrace());
			warnings.push_back(e);
			return;
		}
		running = true;
		if (_port>0)
			intern_port = _port;
		boost::asio::ip::tcp::endpoint ep (boost::asio::ip::tcp::v4(), intern_port);
		accpt.open(ep.protocol());
		accpt.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		accpt.bind(ep);
		accpt.listen();
	}
	
	void WS_Acceptor::close()
	{
		running = false;
		accpt.close();
	}

	bool WS_Acceptor::opened() const
	{
		return running;
	}

	bool WS_Acceptor::full() const
	{
		return client->status() != EndpointStatus::closed;
	}

	int WS_Acceptor::port() const
	{
		return intern_port;
	}

	WS_Endpoint* WS_Acceptor::release()
	{
		if (!full())
		{
			Exception e ("Ein unverbundener Endpunkt wurde abgegeben");
			e.trace.push_back("WS_Acceptor::release()");
			e.trace.push_back(mytrace());
		}
		WS_Endpoint* out = client;
		client = new WS_Endpoint();
		return out;
	}

	void WS_Acceptor::tick()
	{
		ioc.poll_one();
		client->tick();
		warnings.overtake(client->warnings,mytrace());
		if (!running)
			return;
		if (connected>0)
		{
			time_t now;
			time(&now);
			if (difftime(now,connected)>5)
			{
				Exception e ("Zeitueberschreitung bei Verbindungsaufbau");
				e.trace.push_back(mytrace());
				warnings.push_back(e);
				close();
				open();
				return;
			}
		}
		if (!waiting && connected==0 && !full())
			accept();
	}

	WS_Acceptor::~WS_Acceptor()
	{
		delete client;
	}
}