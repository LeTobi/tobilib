#include "utf8.h"
#include <string>
#include "../general/exception.hpp"

namespace tobilib::utf8
{
	
	int firstLOW (unsigned int byte)
	{
		unsigned int stelle = 0x80;
		int count = 0;
		while (stelle&byte)
		{
			count++;
			stelle = stelle>>1;
		}
		return count;
	}
	
	int lastHIGH (unsigned int byte)
	{
		unsigned int stelle = 1;
		int last = 0;
		int count = 1;
		while (stelle)
		{
			if (byte&stelle)
				last = count;
			count++;
			stelle = stelle<<1;
		}
		return last;
	}
	
	unsigned int readbytevalue(unsigned int byte)
	{
		int off = firstLOW(byte);
		return byte & (0xFF >> (off+1));
	}
	
	int getRequiredLength(unsigned int byte)
	{
		if (byte<0x80)
			return 1;
		else if (byte>0x800000000)
		{
			Exception err ("Ein Byte ist zu gross, um kodiert zu werden.");
			err.trace.push_back("utf8::getRequiredLength()");
			throw err;
		}
		int required = lastHIGH(byte);
		int give = 1;
		while (give*5+1<required)
			give++;
		return give;
	}
	
	StringPlus encode(const StringPlus& txt)
	{
		StringPlus out;
		for (int i=0;i<txt.size();i++)
		{
			wchar_t c = txt[i];
			int len = getRequiredLength(c);
			if (len==1)
			{
				out+=c;
				continue;
			}
			int byte = (0xFF << (8-len)) & 0xFF;
			byte = byte | (c>>(6*(len-1)));
			out += byte;
			for (int i=1;i<len;i++)
			{
				byte = 0x80 | (0x3F & (c>>(6*(len-i-1))));
				out+=(int)byte;
			}
		}
		return out;
	}
	
	StringPlus decode(const StringPlus& code)
	{
		StringPlus out;
		unsigned int pos = 0;
		while (pos<code.size())
		{
			unsigned int newbyte = 0;
			int len = firstLOW(code[pos]);
			if (len==1)
			{
				Exception err ("ungueltiges Startbyte an der stelle ");
				err+=std::to_string(pos);
				err.trace.push_back("utf8::decode()");
				throw err;
			}
			len = len==0?0:len-1;
			if (code.size()<=pos+len)
			{
				Exception err ("Der Code endet unerwartet");
				err.trace.push_back("utf8::decode()");
				throw err;
			}
			int off = 0;
			for (int i=len;i>=0;i--)
			{
				newbyte = newbyte | (readbytevalue(code[pos+i])<<off);
				off+=6;
			}
			out += newbyte;
			pos+=len+1;
		}
		return out;
	}
}