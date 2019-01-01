#ifndef TC_NETWORK_SERVER
#define TC_NETWORK_SERVER

#include "acceptor.h"

namespace tobilib::stream
{
    template <class X_Acceptor, class DataT>
    class Server
    {
    public:
        typedef X_Acceptor AcceptorType;
        typedef typename X_Acceptor::EndpointType EndpointType;
        typedef DataT DataType;

        struct Connection
        {
           EndpointType* endpoint;
           DataType data;
        };

        typedef std::set<Connection*> Connection_set;

        Server();
        Server(int);
        Server(const Server&) = delete;
        void operator=(const Server&) = delete;
        ~Server();

        const Connection_set& clients() const;
        void start(int _port=0);
        void stop();
        void tick();
        bool running() const;
        int port() const;
        void remove(Connection*);

        Warning_list warnings;
    private:
        Connection_set connections;
        AcceptorType accpt;

        std::string mytrace() const;
    };

    template <class DataT>
    using WS_Server = Server<WS_Acceptor,DataT>;

    // IMPLEMENTATION /////////////////////////////////

    template <class X_Acceptor, class DataT>
    Server<X_Acceptor,DataT>::Server()
    { }

    template <class X_Acceptor, class DataT>
    Server<X_Acceptor,DataT>::Server(int _port): accpt(_port)
    { }

    template <class X_Acceptor, class DataT>
    const typename Server<X_Acceptor,DataT>::Connection_set& Server<X_Acceptor,DataT>::clients() const
    {
        return connections;
    }

    template <class X_Acceptor, class DataT>
    void Server<X_Acceptor,DataT>::start(int _port)
    {
        accpt.open(_port);
    }

    template <class X_Acceptor, class DataT>
    void Server<X_Acceptor,DataT>::stop()
    {
        accpt.close();
    }

    template <class X_Acceptor, class DataT>
    void Server<X_Acceptor,DataT>::tick()
    {
        accpt.tick();
        while (accpt.filled())
        {
            Connection* conn = new Connection();
            conn->endpoint = accpt.release();
            connections.insert(conn);
        }
        for (auto& conn: connections)
        {
            conn->endpoint->tick();
            warnings.overtake(conn->endpoint->warnings,mytrace());
        }
        warnings.overtake(accpt.warnings,mytrace());
    }

    template <class X_Acceptor, class DataT>
    bool Server<X_Acceptor,DataT>::running() const
    {
        return accpt.status() == X_Acceptor::Status::Open;
    }

    template <class X_Acceptor, class DataT>
    int Server<X_Acceptor,DataT>::port() const
    {
        return accpt.port();
    }

    template <class X_Acceptor, class DataT>
    void Server<X_Acceptor,DataT>::remove (typename Server<X_Acceptor,DataT>::Connection* c)
    {
        if (connections.count(c)==0)
        {
            Exception e ("Die Angegebene Verbindung existiert nicht");
            e.trace.push_back("remove()");
            e.trace.push_back(mytrace());
            warnings.push_back(e);
            return;
        }
        connections.erase(c);
    }

    template <class X_Acceptor, class DataT>
    std::string Server<X_Acceptor,DataT>::mytrace() const
    {
        return std::string("stream::Server Port ")+std::to_string(accpt.port());
    }

    template <class X_Acceptor, class DataT>
    Server<X_Acceptor,DataT>::~Server()
    {
        while (!connections.empty())
        {
            connections.erase(connections.begin());
        }
    }
}

#endif