#ifndef TC_NETWORK_WRITER_H
#define TC_NETWORK_WRITER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "../general/timer.hpp"

namespace tobilib{
namespace network{

    struct WriterOptions
    {
        double send_timeout = 0;
    };

namespace detail{

    // Stream:
    // boost::asio::ip::tcp::socket
    // boost::asio::ssl::stream
    template<class Stream>
    class StreamWriter
    {
    public:
        StreamWriter(WriterOptions&,Stream&);

        void tick();
        void send_data(const std::string&);
        bool is_async() const;
        void reset();

        bool timed_out = false;
        bool written = false;
        boost::system::error_code error;

    private:
        WriterOptions& options;
        Stream& stream;
        
        Timer timer;
        bool async = false;
        std::string data_sending;
        std::string data_queue;

        void on_write(const boost::system::error_code& ec, std::size_t);
    };

    template<class Stream>
    class WS_Writer
    {
    public:
        using WSStream = boost::beast::websocket::stream<Stream>;

        WS_Writer(WriterOptions&,WSStream&);

        void tick();
        void send_data(const std::string&);
        bool is_async() const;
        void reset();

        bool timed_out = false;
        bool written = false;
        boost::system::error_code error;

    private:
        WriterOptions& options;
        WSStream& stream;

        Timer timer;
        bool async = false;
        std::string data_sending;
        std::string data_queue;

        void on_write(const boost::system::error_code&ec, std::size_t);
    };

} // namespace detail
} // namespace network
} // namespace tobilib

#endif