#ifndef TC_H2EP_SERVER
#define TC_H2EP_SERVER

#include "acceptor.h"
#include "../network/server.hpp"

namespace tobilib::h2ep
{
    template <class DataT>
    using WS_Server = stream::Server<WS_Acceptor,DataT>;
}

#endif