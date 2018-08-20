#include "event.h"
#include "error.h"
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
// Für Integer-Parsing:
#include "../stringplus/stringplus.h"

namespace tobilib::h2ep
{
	std::string Event::stringify() const
	{
		std::string out;
		out += "(";
		out += std::to_string(name.size());
		out += ")";
		out += name;
		
		std::stringstream ss;
		boost::property_tree::json_parser::write_json(ss,data,false);
		out += "(";
		out += std::to_string(ss.str().size());
		out += ")";
		out += ss.str();
		return out;
	}
	
	const std::string& Event_parser::buffer() const
	{
		return data;
	}
	
	void Event_parser::feed(const std::string& val)
	{
		data += val;
	}
	
	void Event_parser::getchunks()
	{
		while (chunks.size()<2)
		{
			int start = 0;
			chunk c;
			if (chunks.size()>0)
			{
				chunk last = chunks.back();
				start = last.start+last.size;
			}
			if (data.size()<=start)
				return;
			if (data[start] != '(')
				throw h2parser_error("Fehlformattierung: '(' erwartet.");
			int end = data.find(')',start);
			if (end == std::string::npos)
				return;
			c.start = end+1;
			try {
				c.size = StringPlus(data.substr(start+1,end-start-1)).toInt();
			} catch (StringPlus_error& e) {
				throw h2parser_error("Fehlformattierung: ungueltige Zahl zwischen Klammern");
			}
			chunks.push(c);
		}
	}
	
	bool Event_parser::ready()
	{
		getchunks();
		return chunks.size()>=2;
	}
	
	Event Event_parser::next()
	{
		if (!ready())
			throw h2parser_error("Daten sind nicht vollstaendig.");
		Event ev;
		ev.name = data.substr(chunks.front().start,chunks.front().size);
		chunks.pop();
		std::stringstream ss;
		ss << data.substr(chunks.front().start,chunks.front().size);
		try {
			boost::property_tree::json_parser::read_json(ss,ev.data);
		} catch (boost::property_tree::json_parser_error& e) {
			throw h2parser_error("Das Objekt konnte nicht JSON geparst werden.");
		}
		data.erase(0,chunks.front().start+chunks.front().size);
		chunks.pop();
		return ev;
	}
}