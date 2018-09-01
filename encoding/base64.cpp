#include "base64.h"
#include "error.h"

namespace tobilib
{
	const StringPlus b64char_order = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	StringPlus encode64(const StringPlus& text)
	{
		StringPlus out (0,(text.size()*4+2)/3);
		int overflow = 0;
		for (int i=0;i<text.size();i++)
		{
			if (text[i]>0xFF)
				throw encoding_error("Ein Byte ist laenger als 8 bit");
			int codei = i*4/3;
			out[codei] |= text[i]>>(2+overflow);
			out[codei+1] |= 0x3F & (text[i]<<(4-overflow));
			overflow = (overflow+2)%6;
		}
		for (int i=0;i<out.size();i++)
		{
			out[i] = b64char_order[out[i]];
		}
		out.append((3-text.size()%3)%3,'=');
		return out;
	}
	
	StringPlus decode64(const StringPlus& code)
	{
		if (code.size()%4 != 0)
			throw encoding_error("Der Base64-String ist kein Vielfaches von 4");
		StringPlus out;
		int added = code.count_all('=');
		StringPlus c = code.replace_all("=","A");
		for (int i=0;i<code.size();i+=4)
		{
			int b1 = b64char_order.find(c[i]);
			int b2 = b64char_order.find(c[i+1]);
			int b3 = b64char_order.find(c[i+2]);
			int b4 = b64char_order.find(c[i+3]);
			if (b1==std::string::npos || b2==std::string::npos || b3==std::string::npos || b4==std::string::npos)
				throw encoding_error("Es befindet sich ein ungueltiges Zeichen im Code");
			out += (b1<<2) | b2>>4;
			out += 0xFF & (b2<<4) | (b3>>2);
			out += 0xFF & (b3<<6) | b4;
		}
		return out.substr(0,out.size()-added);
	}
}