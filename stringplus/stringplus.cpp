﻿#include "stringplus.h"
#include <cctype>
#include <fstream>


namespace tobilib
{
	const StringPlus StringPlus::NO_CONTENT = "\0";

	#ifdef ENDLINE
		const StringPlus StringPlus::ENDLINE = ENDLINE;
	#else
		const StringPlus StringPlus::ENDLINE = "\r\n";
	#endif

	StringPlus::StringPlus(): std::u32string() {}

	StringPlus::StringPlus(const char * val)
	{
		assign(val);
	}

	StringPlus::StringPlus(const CharPlus& val, int count)
	{
		assign(val,count);
	}

	StringPlus::StringPlus(const std::string& val)
	{
		assign(val);
	}

	StringPlus::StringPlus(const std::u32string& val) : std::u32string(val)
	{
	}

	void StringPlus::assign(const std::string& str)
	{
		for (int i=0;i<str.size();i++)
		{
			append(1,str.at(i));
		}
	}

	void StringPlus::assign(const char * str)
	{
		assign(std::string(str));
	}

	void StringPlus::assign(const CharPlus& val, int count)
	{
		assign("");
		append(count,val);
	}

	std::string StringPlus::toString() const
	{
		std::string out;
		for (int i=0;i<size();i++)
		{
			out.append(1,at(i));
		}
		return out;
	}

	int StringPlus::toInt() const
	{
		StringPlus val;
		bool negative = beginsWith('-');
		if (negative)
			val = substr(1);
		else
			val = *this;
		for (int i=0;i<val.size();i++)
		{
			if (val[i]<'0' || val[i]>'9')
				throw StringPlus_error(StringPlus("Fehler beim Parsen von Int \"")+val+"\"");
		}
		int ival = atoi(val.toString().c_str());
		return negative?-ival:ival;
	}

	std::vector<StringPlus> StringPlus::split(const StringPlus& delim) const
	{
		std::vector<StringPlus> out;
		out.emplace_back();
		for (int i=0;i<size();i++)
		{
			if (i+delim.size()<=size() && substr(i,delim.size())==delim)
			{
				out.emplace_back();
				i+=delim.size()-1;
			}
			else
			{
				out.back() += at(i);
			}
		}
		for (int i=out.size()-1;i>=0;i--)
		{
			if (out[i].size()<1)
				out.erase(out.begin()+i);
		}
		return out;
	}
	
	int StringPlus::find(const StringPlus& val, int start) const
	{
		return std::u32string::find(val,start);
	}
	
	int StringPlus::rfind(const StringPlus& val, int start) const
	{
		if (start<0)
			start = size()-1;
		return std::u32string::rfind(val,start);
	}
	
	std::vector<int> StringPlus::find_all(const StringPlus& val) const
	{
		std::vector<int> out;
		int lastpos = -1;
		while (true)
		{
			lastpos = find(val,lastpos+1);
			if (lastpos==std::string::npos)
			{
				return out;
			}
			else
			{
				out.push_back(lastpos);
			}
		}
	}

	std::vector<int> StringPlus::find_all_of(const StringPlus& val) const
	{
		std::vector<int> out;
		int lastpos = -1;
		while (true)
		{
			lastpos = find_first_of(val,lastpos+1);
			if (lastpos==std::string::npos)
			{
				return out;
			}
			else
			{
				out.push_back(lastpos);
			}
		}
	}

	int StringPlus::parseHex(const StringPlus& hex)
	{
		std::string ziffern = "0123456789ABCDEF";
		int factor = 1;
		int out = 0;
		for (int i=hex.size()-1;i>=0;i--)
		{
			int index = ziffern.find(hex.at(i));
			if (index==std::string::npos)
				return 0;
			out += factor*index;
			factor*=16;
		}
		return out;
	}

	StringPlus StringPlus::toHex(int c)
	{
		if (c<1)
			return "0";
		int factor = 1;
		while (16*factor<=c)
			factor*=16;
		std::string ziffern = "0123456789ABCDEF";
		StringPlus out;
		while (factor>0)
		{
			int count = 0;
			while (c>=factor)
			{
				c-=factor;
				count++;
			}
			out.append(1,ziffern[count]);
			factor/=16;
		}
		return out;
	}

	StringPlus StringPlus::toLowerCase() const
	{
		StringPlus out;
		for (int i=0;i<size();i++)
		{
			out += tolower(at(i));
		}
		return out;
	}

	StringPlus StringPlus::shrink() const
	{
		StringPlus out (*this);
		while (out.size()>0 && out.at(0)==' ')
		{
			out.erase(0,1);
		}
		while (out.size()>0 && out.at(out.size()-1)==' ')
		{
			out.erase(out.size()-1,1);
		}
		return out;
	}

	StringPlus StringPlus::normalize(const StringPlus& fname) const
	{
		StringPlus table = StringPlus::fromFile(fname);
		if (table == NO_CONTENT)
			return NO_CONTENT;
		StringPlus out = *this;
		std::vector<StringPlus> lines = table.split(ENDLINE);
		for (auto& line: lines)
		{
			std::vector<StringPlus> chars = line.split(" ");
			for (auto& c: chars)
			{
				std::vector<int> indexes = out.find_all(c);
				for (auto pos=indexes.rbegin(); pos!=indexes.rend(); pos++)
				{
					out.erase(*pos,c.size());
					out.insert(*pos,chars[0]);
				}
			}
		}
		return out;
	}

	StringPlus StringPlus::replace_all(const StringPlus& trigger, const StringPlus& subst) const
	{
		StringPlus out = *this;
		std::vector<int> poses = out.find_all(trigger);
		for (int i=poses.size()-1;i>=0;i--)
		{
			out.replace(poses[i],trigger.size(),subst);
		}
		return out;
	}

	StringPlus StringPlus::replace_all_of(const StringPlus& trigger, const StringPlus& subst) const
	{
		StringPlus out = *this;
		std::vector<int> poses = out.find_all_of(trigger);
		for (int i=poses.size()-1;i>=0;i--)
		{
			out.replace(poses[i],1,subst);
		}
		return out;
	}

	bool StringPlus::endsWith(const StringPlus& txt) const
	{
		return substr(size()-txt.size()) == txt;
	}

	bool StringPlus::beginsWith(const StringPlus& txt) const
	{
		return substr(0,txt.size()) == txt;
	}

	int StringPlus::count_all(const StringPlus& needle) const
	{
		int out = 0;
		for (int i=0;i<=size()-needle.size();i++)
		{
			if (substr(i,needle.size()) == needle)
			{
				out++;
				i+=needle.size()-1;
			}
		}
		return out;
	}
	
	int StringPlus::count_all_of(const StringPlus& targets) const
	{
		int out = 0;
		for (int i=0;i<size();i++)
		{
			if (targets.find(at(i)) != std::string::npos)
				out++;
		}
		return out;
	}

	bool StringPlus::consists_of(const StringPlus& chars) const {
		for (auto& c: *this)
			if (chars.find(c)==npos)
				return false;
		return true;
	}
	
	bool StringPlus::nameCompare (const StringPlus& a, const StringPlus& b, const StringPlus& conversion_table)
	{
		StringPlus strA = a;
		StringPlus strB = b;
		if (conversion_table!=NO_CONTENT)
		{
			StringPlus tmpA = strA.normalize(conversion_table);
			StringPlus tmpB = strB.normalize(conversion_table);
			if (tmpA!=NO_CONTENT && tmpB!=NO_CONTENT)
			{
				strA=tmpA;
				strB=tmpB;
			}
		}
		std::vector<StringPlus> A = strA.toLowerCase().split(" ");
		std::vector<StringPlus> B = strB.toLowerCase().split(" ");
		if (A.size()==0 || B.size()==0)
			return false;
		std::vector<StringPlus>* more = A.size()>B.size()?&A:&B;
		std::vector<StringPlus>* less = A.size()>B.size()?&B:&A;
		for (auto& item: *less)
		{
			bool found = false;
			for (auto& vorgabe: *more)
			{
				if (vorgabe==item)
				{
					found=true;
					break;
				}
			}
			if (!found)
				return false;
		}
	}

	StringPlus StringPlus::fromFile (const StringPlus& fname)
	{
		std::fstream fs (fname.toString().c_str(),std::fstream::in | std::fstream::binary);
		if (!fs.good())
		{
			fs.close();
			throw StringPlus_error(StringPlus("Datei konnte nicht gelesen werden: ")+fname);
		}
		StringPlus out;
		while (fs.good())
		{
			int c = fs.get();
			if (c!=EOF)
				out += c;
		}
		fs.close();
		return out;
	}

	void StringPlus::toFile(const StringPlus& content, const StringPlus& fname)
	{
		std::fstream fs(fname.toString().c_str(),std::fstream::out | std::fstream::binary);
		if (!fs.good())
			throw StringPlus_error(StringPlus("Datei konnte nicht geoeffnet werden: ")+fname);
		std::string block = content.toString();
		fs.write(block.c_str(),block.size());
		if (!fs.good())
			throw StringPlus_error(StringPlus("Datei konnte nicht bearbeitet werden: ")+fname);
	}
}