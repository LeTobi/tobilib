#ifndef TC_PROTOCOL_PARSER_H2RFP
#define TC_PROTOCOL_PARSER_H2RFP

#include <boost/property_tree/ptree.hpp>
#include "../../general/queue.hpp"
#include "../../stringplus/stringplus.h"
#include "../../general/exception.hpp"

namespace tobilib {
namespace h2rfp {

    using JSObject = boost::property_tree::ptree;

    class Message {
    public:
        std::string name;
        unsigned int id;
        JSObject data;

        std::string to_string() const;
    };

namespace detail {

    class Parser {
    public:
        Logger log = std::string("h2rfp Parser: ");
        Queue<Message> output;
        
        bool is_good() const;
        void feed(std::string);
        void reset();

    private:
        enum class State { start, name, id, size, data, error };

        State parseState = State::start;
        Message current;
        unsigned int datalen;
        StringPlus buffer;
    };

} // namespace detail
} // namespace h2rfp
} // namespace tobilib

#endif