#ifndef TCENCRYPT_H
#define TCENCRYPT_H

#include "../stringplus/stringplus.h"

namespace tobilib
{
	class Keycontext
	{
	public:
		int datalen;
		StringPlus password;
		
		int* map;
		
		Keycontext (StringPlus, int);
		~Keycontext();
		
	private:
		int plainpos = -1;
		int codepos = 0;
		
		void advance();
	};
	
	StringPlus tcdecrypt(StringPlus code,StringPlus pass);
	StringPlus tcencrypt(StringPlus text,StringPlus pass);
}

#ifdef TC_AS_HPP
	#include "tcencrypt.cpp"
#endif

#endif