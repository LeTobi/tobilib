#include "tcencrypt.h"

namespace tobilib
{
	const unsigned int EMPTY = -1;

	Keycontext::Keycontext(StringPlus _p, int _l)
	{
		datalen = _l;
		password = _p;
		if (datalen==0)
			return;
		if (password.size()<1)
			password = StringPlus(0,1);
		map = new int[datalen];
		for (int i=0;i<datalen;i++) map[i]=EMPTY;
		do
		{
			advance();
			map[codepos] = plainpos;
		} while (plainpos<datalen-1);
	}

	Keycontext::~Keycontext()
	{
		if (datalen>0)
		{
			delete map;
		}
	}

	void Keycontext::advance()
	{
		plainpos++;
		int offset = password[plainpos%password.size()];
		for (int i=0;i<std::max(offset,1);i++)
		{
			do codepos=(codepos+1)%datalen; while (map[codepos]!=EMPTY);
		}
	}

	StringPlus tcdecrypt(StringPlus code, StringPlus pass)
	{
		StringPlus out (0,code.size());
		Keycontext ctx (pass,code.size());
		for (int i=0;i<out.size();i++)
		{
			out[i] = code[ctx.map[i]];
		}
		return out;
	}

	StringPlus tcencrypt(StringPlus text, StringPlus pass)
	{
		StringPlus out (0,text.size());
		Keycontext ctx (pass, text.size());
		for (int i=0;i<out.size();i++)
		{
			out[ctx.map[i]] = text[i];
		}
		return out;
	}
}