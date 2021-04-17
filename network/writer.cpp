#include "writer-impl.hpp"

template class SocketWriter<boost::asio::ip::tcp::socket>;
template class WebsocketWriter<boost::asio::ip::tcp::socket>;
