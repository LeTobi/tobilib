#ifndef TC_NETWORK_READER_H
#define TC_NETWORK_READER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "../general/timer.hpp"

namespace tobilib{
namespace network{

    struct ReaderOptions
    {
        double read_timeout = 0;
        double inactive_warning = 0;
    };
    
namespace detail{

    // Stream:
    // boost::asio::ip::tcp::socket
    // boost::asio::ssl::stream
    template<class Stream>
    class StreamReader
    {
    public:
        StreamReader(ReaderOptions&,Stream&);

        void tick();
        void start_reading();
        bool is_async() const;
        void reset();

        bool warning = false;
        bool inactive = false;
        bool received = false;
        boost::system::error_code error;
        std::string data;

        const static std::size_t BUFFER_SIZE = 256;

    private:
        ReaderOptions& options;
        Stream& stream;
        Timer timer_A; // used for no activity warning
        Timer timer_B; // used for no activity deadline
        std::string buffer = std::string(BUFFER_SIZE,0);
        bool async = false;

        void on_receive(const boost::system::error_code&,size_t);
    };

    template <class Stream>
    class WS_Reader
    {
    public:
        using WSStream = boost::beast::websocket::stream<Stream>;

        WS_Reader(ReaderOptions&,WSStream&);

        void tick();
        void start_reading();
        bool is_async() const;
        void reset();

        bool warning = false;
        bool inactive = false;
        bool received = false;
        boost::system::error_code error;
        std::string data;

    private:
        ReaderOptions& options;
        WSStream& stream;
        Timer timer_A;
        Timer timer_B;
        boost::asio::streambuf buffer;
        bool async = false;

        void on_receive(const boost::system::error_code&, size_t);
    };

} // namespace detail
} // namespace network
} // namespace tobilib

#endif