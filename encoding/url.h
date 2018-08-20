#ifndef TC_URL_H
#define TC_URL_H

#include "../stringplus/stringplus.h"

namespace tobilib
{
	StringPlus encodeURL(const StringPlus&);
	StringPlus decodeURL(const StringPlus&);
}

#ifdef TC_AS_HPP
	#include "url.cpp"
#endif

#endif