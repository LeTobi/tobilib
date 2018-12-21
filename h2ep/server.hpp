#ifndef TC_H2EP_SERVER
#define TC_H2EP_SERVER

#include "acceptor.h"
#include <list>

namespace tobilib::h2ep
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

        typedef std::list<Connection*> Connection_list;

        Server();
        Server(int);
        Server(const Server&) = delete;
        void operator=(const Server&) = delete;
        ~Server();

        const Connection_list& clients() const;
        void start(int _port=0);
        void stop();
        void tick();
        bool running() const;
        int port() const;
        void remove(Connection*);

        Warning_list warnings;
    private:
        Connection_list connections;
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
    const typename Server<X_Acceptor,DataT>::Connection_list& Server<X_Acceptor,DataT>::clients() const
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
        while (accpt.full())
        {
            Connection* conn = new Connection();
            conn->endpoint = accpt.release();
            connections.push_back(conn);
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
        return accpt.opened();
    }

    template <class X_Acceptor, class DataT>
    int Server<X_Acceptor,DataT>::port() const
    {
        return accpt.port();
    }

    template <class X_Acceptor, class DataT>
    void Server<X_Acceptor,DataT>::remove (typename Server<X_Acceptor,DataT>::Connection* c)
    {
        typename Server<X_Acceptor,DataT>::Connection_list::iterator toremove;
        bool exists = false;
        for (auto _c = connections.begin();_c!=connections.end();_c++)
        {
            if (*_c==c)
            {
                exists=true;
                toremove = _c;
                break;
            }
        }
        if (!exists)
        {
            Exception err ("Die Angegebene Verbindung ist nicht Teil des Servers");
            err.trace.push_back("Server::remove()");
            err.trace.push_back(mytrace());
            warnings.push_back(err);
            return;
        }
        connections.erase(toremove);
        delete c->endpoint;
        delete c;
    }

    template <class X_Acceptor, class DataT>
    std::string Server<X_Acceptor,DataT>::mytrace() const
    {
        return std::string("h2ep::Server Port ")+std::to_string(accpt.port());
    }

    template <class X_Acceptor, class DataT>
    Server<X_Acceptor,DataT>::~Server()
    {
        while (!connections.empty())
        {
            remove(connections.back());
        }
    }
}

#endif