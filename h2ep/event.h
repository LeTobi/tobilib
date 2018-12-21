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
		struct chunk {
			int start;
			int size;
		};
	
		std::string data;
		std::queue<chunk> chunks;
		std::queue<Event> outqueue;
		void getchunks();
		void chunkparse();

	public:
		const std::string& buffer() const;
		void feed(const std::string&);
		bool ready() const;
		Event next();
	};
}

#ifdef TC_AS_HPP
	#include "event.cpp"
#endif

#endif