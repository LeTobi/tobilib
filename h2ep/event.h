#ifndef H2EVENT_H
#define H2EVENT_H

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <queue>

namespace tobilib::h2ep
{
	typedef boost::property_tree::ptree JSObject;
	
	class Event
	{
	public:
		Event(){};
		Event(const std::string& n): name(n) {};
		std::string stringify() const;
		
		std::string name;
		JSObject data;
	};
	
	class Event_parser
	{
	private:
		enum class ChunkStatus {Void,Int,Data};

		ChunkStatus chunk_status = ChunkStatus::Void;
		int chunk_dataleft = 0;
		std::string chunk_buffer;
		std::queue<std::string> chunk_output;

		void chunk_feed(const char);

		std::queue<Event> event_output;
		void event_feed(const char);

	public:
		void feed(const std::string&);
		void clear();
		bool ready() const;
		Event next();
	};
}

#ifdef TC_AS_HPP
	#include "event.cpp"
#endif

#endif