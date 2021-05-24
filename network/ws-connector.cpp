#include "ws-connector-impl.hpp"

template class WS_ClientConnector<WS_Socket,TCP_ClientConnector>;
template class WS_ServerConnector<WS_Socket,TCP_ServerConnector>;

