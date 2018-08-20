#ifndef H2EVENT_H
#define H2EVENT_H

#include "../stringplus/stringplus.h"
#include <boost/property_tree/ptree.hpp>
#include <queue>

namespace tobilib
{
	typedef boost::property_tree::ptree JSObject;
	
	class H2Event
	{
	public:
		H2Event(){};
		H2Event(StringPlus n): name(n) {};
		StringPlus stringify() const;
		
		StringPlus name;
		JSObject data;
	};
	
	class H2Event_parser
	{
	private:
		struct chunk {
			int start;
			int size;
		};
	
		StringPlus data;
		std::queue<chunk> chunks;
		void getchunks();
		
	public:
		const StringPlus& buffer() const;
		void feed(const StringPlus&);
		bool ready();
		H2Event next();
	};
}

#ifdef TC_AS_HPP
	#include "h2event.cpp"
#endif

#endif