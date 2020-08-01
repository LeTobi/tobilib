#include "errors.h"

using namespace tobilib;
using namespace network;

namespace error_detail {
    const std::string messages[] = {
        "success",
        "Handshake-Timeout"
    };

    const char * category_name = "tobilib";
}

ErrorCategory tobilib::network::TobilibErrors;

const char* ErrorCategory::name() const BOOST_NOEXCEPT
{
    return error_detail::category_name;
}

std::string ErrorCategory::message(int code) const BOOST_NOEXCEPT
{
    return error_detail::messages[code];
}