#ifndef TC_WEBSOCKET_READER_H
#define TC_WEBSOCKET_READER_H

#include <boost/beast.hpp>
#include "../../general/timer.hpp"

namespace tobilib {
namespace detail {

    struct WS_reader_options
    {
        double inactive_warning = 0;
        double inactive_shutdown = 0;
    };

    enum class WS_reader_event
    {
        data_arrived,
        inactive_warning,
        inactive_shutdown,
        read_error,
        shutdown
    };

    class WS_Reader
    {
    public:
        WS_Reader(boost::beast::websocket::stream<boost::asio::ip::tcp::socket>&, WS_reader_options*);

        void tick();
        bool is_inactive() const;
        
        void start_reading();
        void reset();
        std::string data;
        std::queue<WS_reader_event> events;

    private:
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket>& socket;
        WS_reader_options* options;
        boost::asio::streambuf buffer;
        Timer inactive_timer;
        Timer deadline_timer;
        bool reading = false;
        bool inactive = false;
        void on_data_read(const boost::system::error_code&, size_t);
    };

} // namespace detail
} // namespace tobilib

#endif