#include "acceptor.h"

namespace tobilib::h2ep
{
	template <class StrAccpt>
	Acceptor<StrAccpt>::Acceptor(Process& ioc, int port): accpt(ioc,port)
	{
		accpt.on_error.notify([this](const network_error& err)
		{
			on_error(protocol_error(err.what()));
		});
		accpt.on_accept.notify(std::bind(&Acceptor<StrAccpt>::intern_accept,this,std::placeholders::_1));
	}
	
	template <class StrAccpt>
	void Acceptor<StrAccpt>::intern_accept(stream::Endpoint* ep)
	{
		Endpoint* pep = new Endpoint(ep);
		on_accept(pep);
	}
	
	template <class StrAccpt>
	void Acceptor<StrAccpt>::next()
	{
		accpt.next();
	}
	
	template class Acceptor<stream::WS_Acceptor>;
}