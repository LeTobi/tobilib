#ifndef TC_NETWORK_SERVER
#define TC_NETWORK_SERVER

#include "./acceptor.h"
#include <set>

namespace tobilib
{
    template <class DataT, class EndpointT>
    class Connection
    {
    public:
        EndpointT* endpoint = NULL;
        DataT data;

        Connection(){};
        Connection(EndpointT* ep): endpoint(ep){};
    };

    template <class DataT, class EndpointT>
    class ConnectionList: std::set<Connection<DataT, EndpointT>*>
    {
    public:
        static std::set<ConnectionList<DataT, EndpointT>*> tracker;

        ConnectionList(const ConnectionList<DataT,EndpointT>&) = delete;
        void operator=(const ConnectionList<DataT,EndpointT>&) = delete;

        ConnectionList()
        {
            tracker.insert(this);
        }

        ~ConnectionList()
        {
            tracker.erase(this);
        }
    };
}

namespace tobilib::stream
{
    template<class DataT>
    using Connection = tobilib::Connection<DataT,Endpoint>;

    template<class AcceptorT, class DataT>
    class Server
    {
    public:

    };

    template<class DataT>
    using WS_Server = Server<WS_Acceptor,DataT>;
}

#endif