#ifndef TC_NETWORK_WRITER_H
#define TC_NETWORK_WRITER_H

#include "alias.h"
#include "../general/timer.hpp"

namespace tobilib{
namespace network{

    struct WriterOptions
    {
        double send_timeout = 0;
    };

namespace detail{

    // SocketType:
    // boost::asio::ip::tcp::socket
    // boost::asio::ssl::stream
    template<class SocketType>
    class SocketWriter
    {
    public:
        SocketWriter(WriterOptions&,SocketType*);

        void tick();
        void send_data(const std::string&);
        bool is_writing() const;
        void reset(SocketType*);

        bool timed_out = false;
        bool written = false;
        boost::system::error_code error;

    private:
        WriterOptions& options;
        SocketType* socket;
        
        Timer timer;
        bool asio_writing = false;
        std::string data_sending;
        std::string data_queue;

        void on_write(const boost::system::error_code& ec, std::size_t);
    };

    template<class SocketType>
    class WebsocketWriter
    {
    public:
        using WebsocketType = boost::beast::websocket::stream<SocketType>;

        WebsocketWriter(WriterOptions&,WebsocketType*);

        void tick();
        void send_data(const std::string&);
        bool is_writing() const;
        void reset(WebsocketType*);

        bool timed_out = false;
        bool written = false;
        boost::system::error_code error;

    private:
        WriterOptions& options;
        WebsocketType* socket;

        Timer timer;
        bool asio_writing = false;
        std::string data_sending;
        std::string data_queue;

        void on_write(const boost::system::error_code&ec, std::size_t);
    };

} // namespace detail
} // namespace network
} // namespace tobilib

#endif