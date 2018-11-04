#include "endpoint.h"

namespace tobilib::h2ep
{
	Endpoint::Endpoint(stream::Endpoint* ep)
	{
		dock(ep);
	}
	
	Endpoint::~Endpoint()
	{
		if (stream!=NULL)
			delete stream;
	}

	void Endpoint::intern_on_error(const network_error& err)
	{
		on_error(protocol_error(err.what()));
	}
	
	void Endpoint::intern_on_receive()
	{
		parser.feed(stream->received);
		stream->received.clear();
		try
		{
			while (parser.ready())
			{
				Event ev = parser.next();
				call(ev);
			}
		}
		catch (h2parser_error& err)
		{
			on_error(protocol_error(std::string("Parsing-Fehler: ")+err.what()));
			stream->close();
		}
	}
	
	void Endpoint::intern_on_close()
	{
		on_close();
	}
	
	void Endpoint::call(const Event& ev)
	{
		if (callbacks.count(ev.name)>0)
		{
			callbacks.at(ev.name)(ev.data);
			return;
		}
		fallback(ev);
	}
	
	void Endpoint::dock(stream::Endpoint* str)
	{
		if (stream != NULL)
		{
			throw protocol_error("Ein h2ep::Endpoint kann nur einmal gedockt werden!");
			return;
		}
		str->on_error.notify(std::bind(&Endpoint::intern_on_error,this,std::placeholders::_1));
		str->on_receive.notify(std::bind(&Endpoint::intern_on_receive,this));
		str->on_close.notify(std::bind(&Endpoint::intern_on_close,this),callback_position::late);
		stream = str;
	}
	
	bool Endpoint::connected() const
	{
		return stream!=NULL && stream->connected();
	}
	
	bool Endpoint::busy() const
	{
		return stream!=NULL && stream->busy();
	}
	
	void Endpoint::send(const Event& ev)
	{
		if (stream == NULL)
		{
			throw protocol_error("Der h2ep::Endpoint wurde nicht an einen Datenstrom gedockt.");
			return;
		}
		try {
			stream->write(ev.stringify());
		} catch (h2parser_error& err) {
			on_error(protocol_error(err.what()));
		}
	}
	
	Callback_Ticket Endpoint::addEventListener(const std::string& name, event_callback cb, callback_position pos)
	{
		if (callbacks.count(name)==0)
		{
			callbacks.emplace(std::piecewise_construct,std::forward_as_tuple(name),std::forward_as_tuple());
		}
		return callbacks[name].notify(cb,pos);
	}
	
	void Endpoint::close()
	{
		if (stream==NULL)
		{
			throw protocol_error("Der h2ep::Endpoint wurde nicht an einen Datenstrom gedockt.");
			return;
		}
		stream->close();
	}
}