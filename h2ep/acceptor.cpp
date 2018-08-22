#include "acceptor.h"

namespace tobilib::h2ep
{
	template <class StrAccpt>
	Acceptor<StrAccpt>::Acceptor(boost::asio::io_context& ioc, int port): accpt(ioc,port)
	{
		accpt.on_error.notify([this](const network_error& err)
		{
			on_error(protocoll_error(err.what()));
		});
		accpt.on_accept.notify(std::bind(&Acceptor<StrAccpt>::intern_accept,this,std::placeholders::_1));
	}
	
	template <class StrAccpt>
	void Acceptor<StrAccpt>::intern_accept(stream::Endpoint& ep)
	{
		Endpoint* pep = new Endpoint(&ep);
		pep->on_close.notify([pep](){
			delete pep;
		},callback_position::late);
		on_accept(*pep);
	}
	
	template <class StrAccpt>
	void Acceptor<StrAccpt>::next()
	{
		accpt.next();
	}
	
	template class Acceptor<stream::WS_Acceptor>;
}