#include "ph2rfp.h"

#include <sstream>
#include <boost/property_tree/json_parser.hpp>

using namespace tobilib;
using namespace h2rfp;
using namespace detail;

std::string Message::to_string() const
{
    std::string output = "!";
    output += name;
    output += ";";
    output += std::to_string(id);
    output += ";";
    
    std::stringstream sstr;
    boost::property_tree::write_json(sstr,data,false); // could but should not throw json_parser_error
    output += std::to_string(sstr.str().size() - 1); // \n is appended for whatever reason
    output += ";";
    output += sstr.str();
    output.erase(output.size()-1,1); // remove \n
    return output;
}

bool Parser::is_good() const
{
    return parseState!=State::error;
}

void Parser::feed(std::string data)
{
    for (int i=0;i<data.size();i++)
    switch(parseState)
    {
    case State::error:
        return;

    case State::start:
        if (data[i]!='!')
        {
            log << "Es wird '!' am Anfang einer Anfrage erwartet" << std::endl;
            parseState = State::error;
            return;
        }
        parseState = State::name;
        break;

    case State::name:
        if (data[i] == ';')
            parseState = State::id;
        else
            current.name += data[i];
        break;

    case State::id:
        if (data[i] == ';')
        {
            int input = -1;
            if (buffer.isInt())
                input = buffer.toInt();
            if (input<0)
            {
                log << "Es wurde eine positive ID erwartet." << std::endl;
                parseState = State::error;
                return;
            }
            current.id = input;
            buffer.clear();
            parseState = State::size;
        }
        else
        {
            buffer += data[i];
        }
        break;

    case State::size:
        if (data[i] == ';')
        {
            int input = -1;
            if (buffer.isInt())
                input = buffer.toInt();
            if (input<0)
            {
                log << "Es wurde eine Datenlaenge erwartet" << std::endl;
                parseState = State::error;
                return;
            }
            datalen = input;
            buffer.clear();
            parseState = State::data;

            if (datalen == 0)
            {
                output.push(current);
                current.name.clear();
                current.id = 0;
                current.data.clear();
                parseState = State::start;
            }
        }
        else
        {
            buffer += data[i];
        }
        break;

    case State::data:
        buffer += data[i];
        if (buffer.size() == datalen)
        {
            std::stringstream stream (buffer);
            try {
                boost::property_tree::json_parser::read_json(stream,current.data);
            } catch (boost::property_tree::json_parser_error err) {
                log << err.what() << std::endl;
                parseState = State::error;
                return;
            }
            buffer.clear();
            output.push(current);
            current.name.clear();
            current.id = 0;
            current.data.clear();
            parseState = State::start;
        }
        break;
    }
}

void Parser::reset()
{
    output.clear();
    parseState = State::start;
    current.name.clear();
    current.id = 0;
    current.data.clear();
    datalen = 0;
    buffer.clear();
}