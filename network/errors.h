#ifndef TC_NETWORK_ERRORS_H
#define TC_NETWORK_ERRORS_H

#include <boost/system/error_code.hpp>
#include <string>

namespace tobilib {
namespace network {

    class ErrorCategory: public boost::system::error_category
    {
    public:
        const char* name() const BOOST_NOEXCEPT;
        std::string message(int) const BOOST_NOEXCEPT;

        enum {
            success = 0,
            handshake_timeout
        };
    };

    extern ErrorCategory TobilibErrors; 
}
}

#endif