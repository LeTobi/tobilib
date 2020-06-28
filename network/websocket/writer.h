#ifndef TC_WEBSOCKET_WRITER_H
#define TC_WEBSOCKET_WRITER_H

#include <boost/beast.hpp>
#include "../../general/queue.hpp"
#include "../../general/timer.hpp"

namespace tobilib {
namespace detail {

    struct WS_writer_options
    {
        double send_timeout = 0;
        double close_timeout = 10;
    };

    enum class WS_writer_event
    {
        send_timeout,
        close_timeout,
        send_error
    };

    class WS_Writer
    {
    public:
        WS_Writer(boost::beast::websocket::stream<boost::asio::ip::tcp::socket>&, WS_writer_options&);

        void tick();
        void send_data(const std::string&);
        void send_close();
        bool is_busy() const;
        void reset();

        Queue<WS_writer_event> events;
    private:
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket>& socket;
        WS_writer_options& options;
        std::string data_queue;
        std::string data_sending;
        bool writing = false;
        bool closing = false;
        bool wannaclose = false;
        Timer timeout;
        void flush();
        void on_data_written(const boost::system::error_code&, size_t);
        void on_close_written(const boost::system::error_code&);
    };

} // namespace detail
} // namespace tobilib

#endif