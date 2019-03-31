#include "event.h"
#include "../general/exception.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

// Integer-Parsing:
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
		try {
			boost::property_tree::json_parser::write_json(ss,data,false);
		} catch (boost::property_tree::json_parser_error& e) {
			Exception err (e.message());
			err.trace.push_back("h2ep::Event::stringify()");
			throw err;
		}
		out += "(";
		out += std::to_string(ss.str().size());
		out += ")";
		out += ss.str();
		return out;
	}
	
	void Event_parser::chunk_feed(const char c)
	{
		switch (chunk_status)
		{
		case ChunkStatus::Void:
			if (c!='(')
			{
				Exception err ("Das H2-Event-Protokoll erwartet ein '('");
				err.trace.push_back("Event_parser::chunk_feed()");
				clear();
				throw err;
			}
			chunk_status = ChunkStatus::Int;
			break;
		
		case ChunkStatus::Int:
			if (c==')')
			{
				try {
					chunk_dataleft = StringPlus(chunk_buffer).toInt();
				} catch (Exception& err) {
					err.trace.push_back("Event_parser::chunk_feed()");
					clear();
					throw err;
				}
				chunk_buffer.clear();
				chunk_status = ChunkStatus::Data;
			}
			else
			{
				chunk_buffer+=c;
			}
			break;
		
		case ChunkStatus::Data:
			chunk_buffer += c;
			chunk_dataleft--;
			if (chunk_dataleft<=0)
			{
				chunk_output.push(chunk_buffer);
				chunk_buffer.clear();
				chunk_status = ChunkStatus::Void;
			}
			break;
		}
	}

	void Event_parser::event_feed(const char c)
	{
		chunk_feed(c);
		while (chunk_output.size()>=2)
		{
			Event ev;
			ev.name = chunk_output.front();
			chunk_output.pop();
			std::stringstream ss (chunk_output.front());
			chunk_output.pop();
			try {
				boost::property_tree::json_parser::read_json(ss,ev.data);
			} catch (boost::property_tree::json_parser_error& e) {
				clear();
				Exception err("Das Objekt konnte nicht JSON geparst werden.");
				err.trace.push_back("Event_parser::event_feed()");
				throw err;
			}
			event_output.push(ev);
		}
	}

	void Event_parser::feed(const std::string& val)
	{
		for (auto& c: val)
		{
			event_feed(c);
		}
	}

	void Event_parser::clear()
	{
		chunk_dataleft = 0;
		chunk_buffer.clear();
		while (!chunk_output.empty())
			chunk_output.pop();
		chunk_status = ChunkStatus::Void;
		while (!event_output.empty())
			event_output.pop();
	}

	bool Event_parser::ready() const
	{
		return !event_output.empty();
	}
	
	Event Event_parser::next()
	{
		if (!ready())
		{
			Exception err ("Keine Daten vorhanden");
			err.trace.push_back("Event_parser::next()");
			throw err;
		}
		Event out = event_output.front();
		event_output.pop();
		return out;
	}
}