#include "h2event.h"
#include "error.h"
#include <sstream>
#include <boost/property_tree/json_parser.hpp>

namespace tobilib
{
	StringPlus H2Event::stringify() const
	{
		StringPlus out;
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
	
	const StringPlus& H2Event_parser::buffer() const
	{
		return data;
	}
	
	void H2Event_parser::feed(const StringPlus& val)
	{
		data += val;
	}
	
	void H2Event_parser::getchunks()
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
	
	bool H2Event_parser::ready()
	{
		getchunks();
		return chunks.size()>=2;
	}
	
	H2Event H2Event_parser::next()
	{
		if (!ready())
			throw h2parser_error("Daten sind nicht vollstaendig.");
		H2Event ev;
		ev.name = data.substr(chunks.front().start,chunks.front().size);
		chunks.pop();
		std::stringstream ss;
		ss << StringPlus(data.substr(chunks.front().start,chunks.front().size)).toString();
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