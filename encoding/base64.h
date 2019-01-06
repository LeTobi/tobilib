#ifndef TC_BASE64_H
#define TC_BASE64_H

#include "../stringplus/stringplus.h"

namespace tobilib::base64
{
	StringPlus encode (const StringPlus&);
	StringPlus decode (const StringPlus&);
}

#ifdef TC_AS_HPP
	#include "base64.cpp"
#endif

#endif