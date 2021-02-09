#include "url.h"

namespace tobilib
{
	StringPlus decodeURL(const StringPlus& url)
	{
		std::string ziffern = "0123456789ABCDEF";
		StringPlus out;
		for (int i=0;i<url.size();i++)
		{
			if (url[i]=='%')
			{
				if (url.size()-i<3)
					return out;
				out += url.substr(i+1,2).toInt_Hex();
				i+=2;
			}
			else
			{
				out += url[i];
			}
		}
		return out;
	}

	StringPlus encodeURL(const StringPlus& text)
	{
		std::string normalzeichen = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		StringPlus out;
		for (int i=0;i<text.size();i++)
		{
			if (normalzeichen.find(text[i])==std::string::npos)
			{
				out.append(1,'%');
				if (text[i]<0x10)
					out.append(1,'0');
				out.append(StringPlus::make_hex(text[i]));
			}
			else
			{
				out.append(1,text[i]);
			}
		}
		return out;
	}
}