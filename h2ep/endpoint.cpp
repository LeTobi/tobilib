#include "endpoint.h"

namespace tobilib::h2ep
{
	template <class StrEndp>
	Endpoint<StrEndp>::Endpoint(StrEndp* ep, bool _responsable): stream(ep), responsable(_responsable)
	{ }
	
	template <class StrEndp>
	Endpoint<StrEndp>::~Endpoint()
	{
		if (responsable)
			delete stream;
	}
	
	template <class StrEndp>
	void Endpoint<StrEndp>::tick()
	{
		stream->tick();
		try
		{
			if (stream->readable())
				parser.feed(stream->read());
		}
		catch (Exception& err)
		{
			err.trace.push_back("h2ep::Event feed()");
			err.trace.push_back(mytrace());
			warnings.push_back(err);
			close();
		}
		warnings.overtake(stream->warnings,mytrace());
	}

	template <class StrEndp>
	bool Endpoint<StrEndp>::readable() const
	{
		return parser.ready();
	}

	template <class StrEndp>
	Event Endpoint<StrEndp>::read()
	{
		if (parser.ready())
			return parser.next();
		return Event();
	}
	
	template <class StrEndp>
	EndpointStatus Endpoint<StrEndp>::status() const
	{
		return stream->status();
	}
	
	template <class StrEndp>
	bool Endpoint<StrEndp>::inactive() const
	{
		return stream->inactive();
	}

	template <class StrEndp>
	void Endpoint<StrEndp>::inactive_checked()
	{
		return stream->inactive_checked();
	}

	template <class StrEndp>
	bool Endpoint<StrEndp>::busy() const
	{
		return stream->busy();
	}
	
	template <class StrEndp>
	void Endpoint<StrEndp>::send(const Event& ev)
	{
		try {
			stream->write(ev.stringify());
		} catch (Exception& err) {
			warnings.push_back(err);
		}
	}
	
	template <class StrEndp>
	void Endpoint<StrEndp>::close()
	{
		stream->close();
	}

	template <class StrEndp>
	const boost::asio::ip::address& Endpoint<StrEndp>::remote_ip() const
	{
		return stream->remote_ip();
	}

	template<class StrEndp>
	std::string Endpoint<StrEndp>::mytrace() const
	{
		std::string out = "h2ep::Endpoint ";
		boost::asio::ip::address addr = stream->remote_ip();
		if (addr.is_unspecified())
			out+=" nicht verbunden";
		else
			out+=addr.to_string();
		return out;
	}

	template class Endpoint<stream::WS_Endpoint>;
}