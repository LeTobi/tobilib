#include "acceptor.h"

namespace tobilib::h2ep
{
	template <class StrAccpt>
	Acceptor<StrAccpt>::Acceptor()
	{ }

	template <class StrAccpt>
	Acceptor<StrAccpt>::Acceptor(int _port): accpt(_port)
	{ }
	
	template <class StrAccpt>
	void Acceptor<StrAccpt>::tick()
	{
		accpt.tick();
		warnings.overtake(accpt.warnings,mytrace());
	}
	
	template <class StrAccpt>
	void Acceptor<StrAccpt>::open(int _port)
	{
		accpt.open(_port);
	}

	template <class StrAccpt>
	void Acceptor<StrAccpt>::close()
	{
		accpt.close();
	}

	template <class StrAccpt>
	typename Acceptor<StrAccpt>::Status Acceptor<StrAccpt>::status() const
	{
		return accpt.status();
	}

	template <class StrAccpt>
	bool Acceptor<StrAccpt>::filled() const
	{
		return accpt.filled();
	}

	template <class StrAccpt>
	int Acceptor<StrAccpt>::size() const
	{
		return accpt.size();
	}

	template <class StrAccpt>
	int Acceptor<StrAccpt>::port() const
	{
		return accpt.port();
	}

	template <class StrAccpt>
	typename Acceptor<StrAccpt>::EndpointType* Acceptor<StrAccpt>::release()
	{
		return new EndpointType(accpt.release(),true);
	}

	template <class StrAccpt>
	std::string Acceptor<StrAccpt>::mytrace() const
	{
		return std::string("h2ep::Acceptor Port ")+std::to_string(accpt.port());
	}
	
	template class Acceptor<stream::WS_Acceptor>;
}