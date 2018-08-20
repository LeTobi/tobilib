#include "endpoint.h"

namespace tobilib::h2ep
{
	
	void Endpoint::intern_on_error(const network_error& err)
	{
		if (on_error)
			on_error(protocoll_error(err.what()));
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
			if (on_error)
				on_error(protocoll_error(std::string("Parsing-Fehler: ")+err.what()));
			stream->close();
		}
	}
	
	void Endpoint::intern_on_close()
	{
		if (on_close)
			on_close();
	}
	
	void Endpoint::call(const Event& ev)
	{
		if (callbacks.count(ev.name)>0)
		{
			event_callback cb = callbacks.at(ev.name);
			if (cb)
				cb(ev.data);
			return;
		}
		if (fallback)
			fallback(ev);
	}
	
	void Endpoint::dock(stream::Endpoint* str)
	{
		if (stream != NULL)
		{
			throw protocoll_error("Ein h2ep::Endpoint kann nur einmal gedockt werden!");
			return;
		}
		str->on_error = std::bind(&Endpoint::intern_on_error,this,std::placeholders::_1);
		str->on_receive = std::bind(&Endpoint::intern_on_receive,this);
		str->on_close = std::bind(&Endpoint::intern_on_close,this);
		stream = str;
	}
	
	void Endpoint::send(const Event& ev)
	{
		if (stream == NULL)
		{
			throw protocoll_error("Der h2ep::Endpoint wurde nicht an einen Datenstrom gedockt.");
			return;
		}
		stream->write(ev.stringify());
	}
	
	void Endpoint::addEventListener(const std::string& name, event_callback cb)
	{
		callbacks.emplace(name,cb);
	}
	
	void Endpoint::close()
	{
		if (stream==NULL)
		{
			throw protocoll_error("Der h2ep::Endpoint wurde nicht an einen Datenstrom gedockt.");
			return;
		}
		stream->close();
	}
}