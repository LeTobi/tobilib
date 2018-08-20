#ifndef TC_BASE64_H
#define TC_BASE64_H

#include "../stringplus/stringplus.h"

namespace tobilib
{
	StringPlus encode64 (const StringPlus&);
	StringPlus decode64 (const StringPlus&);
}

#ifdef TC_AS_HPP
	#include "base64.cpp"
#endif

#endif