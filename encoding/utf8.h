#ifndef TC_UTF8
#define TC_UTF8

#include "../stringplus/stringplus.h"
#include "error.h"

namespace tobilib::utf8
{	
	int firstLOW (unsigned int byte);
	
	int lastHIGH (unsigned int byte);
	
	unsigned int readbytevalue(unsigned int byte);
	
	int getRequiredLength(unsigned int byte);
	
	StringPlus encode(const StringPlus& txt);
	
	StringPlus decode(const StringPlus& code);
}

#ifdef TC_AS_HPP
	#include "utf8.cpp"
#endif

#endif