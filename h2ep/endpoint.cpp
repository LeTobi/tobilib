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
	typename Endpoint<StrEndp>::Status Endpoint<StrEndp>::status() const
	{
		return stream->status();
	}

	template <class StrEndp>
	void Endpoint<StrEndp>::reactivate(const Event& ev)
	{
		try {
			stream->reactivate(ev.stringify());
		} catch (Exception& err) {
			warnings.push_back(err);
		}
	}

	template <class StrEndp>
	bool Endpoint<StrEndp>::write_busy() const
	{
		return stream->write_busy();
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
	bool Endpoint<StrEndp>::read_available() const
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
	void Endpoint<StrEndp>::shutdown()
	{
		stream->shutdown();
	}

	template <class StrEndp>
	void Endpoint<StrEndp>::tick()
	{
		// Das parser.clear() ist extrem hässlich :/
		bool is_closed = stream->status()==StrEndp::Status::Closed;
		stream->tick();
		if (is_closed && stream->status()!=StrEndp::Status::Closed)
			parser.clear();
		try
		{
			if (stream->read_size()>0)
				parser.feed(stream->read());
		}
		catch (Exception& err)
		{
			err.trace.push_back("h2ep::Event feed()");
			err.trace.push_back(mytrace());
			warnings.push_back(err);
			shutdown();
		}
		warnings.overtake(stream->warnings,mytrace());
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
			out+="keine ip";
		else
			out+=addr.to_string();
		return out;
	}

	template class Endpoint<stream::WS_Endpoint>;
}