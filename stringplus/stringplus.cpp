#include "stringplus.h"
#include "../general/exception.hpp"
#include <cctype>
#include <fstream>
#include <random>
#include <cstdlib>


namespace tobilib
{
	const StringPlus StringPlus::NO_CONTENT = "\0";
	const StringPlus StringPlus::DEFAULT_CHARSET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	const StringPlus StringPlus::HEX_SYMBOLS = "0123456789abcdef";

	#ifdef ENDLINE
		const StringPlus StringPlus::ENDLINE = ENDLINE;
	#else
		const StringPlus StringPlus::ENDLINE = "\r\n";
	#endif

	StringPlus::StringPlus(): std::u32string() {}

	StringPlus::StringPlus(const char * val) {
		assign(val);
	}

	StringPlus::StringPlus(const CharPlus& val, int count) {
		assign(val,count);
	}

	StringPlus::StringPlus(const std::string& val) {
		assign(val);
	}

	StringPlus::StringPlus(const std::u32string& val) : std::u32string(val) {
	}

	void StringPlus::assign(const char * str) {
		resize(0);
		int i=-1;
		while (str[++i]!='\0')
			append(1,str[i]);
	}

	void StringPlus::assign(const std::string& str) {
		assign(str.c_str());
	}

	void StringPlus::assign(const CharPlus& val, int count) {
		resize(0);
		append(count,val);
	}

	StringPlus& StringPlus::insert(int pos, const StringPlus& val) {
		std::u32string::insert(pos,val);
		return *this;
	}

	std::string StringPlus::toString() const {
		std::string out;
		for (int i=0;i<size();i++)
		{
			out.append(1,at(i));
		}
		return out;
	}

	StringPlus StringPlus::numeric_sign() const 
	{
		if (beginsWith("-"))
			return "-";
		if (beginsWith("+"))
			return "+";
		return "";
	}

	StringPlus StringPlus::numeric_prefix() const
	{
		StringPlus x = substr(numeric_sign().size());
		if (x.beginsWith("0x"))
			return "0x";
		if (x.beginsWith("0b"))
			return "0b";
		return "";
	}

	StringPlus StringPlus::numeric_body() const
	{
		return substr(numeric_sign().size()+numeric_prefix().size());
	}

	bool StringPlus::isInt() const
	{
		return isDecimal() ||
			(numeric_prefix()=="0x"&&isHex()) ||
			(numeric_prefix()=="0b"&&isBinary());
	}

	bool StringPlus::isDecimal() const
	{
		if (empty())
			return false;
		if (numeric_prefix()!="")
			return false;
		for (auto& c: numeric_body())
			if (c<'0' || c>'9')
				return false;
		return true;
	}

	bool StringPlus::isHex() const
	{
		if (!StringPlus("0x").beginsWith(numeric_prefix()))
			return false;
		for (auto& c: numeric_body())
			if (HEX_SYMBOLS.find(tolower(c))==npos)
				return false;
		return true;
	}

	bool StringPlus::isBinary() const
	{
		if (!StringPlus("0b").beginsWith(numeric_prefix()))
			return false;
		for (auto& c: numeric_body())
			if (c!='0' && c!='1')
				return false;
		return true;
	}

	int StringPlus::toInt() const
	{
		if (numeric_prefix()=="0b")
			return toInt_Binary();
		if (numeric_prefix()=="0x")
			return toInt_Hex();
		return toInt_Decimal();
	}

	int StringPlus::toInt_Binary() const
	{
		int out = stoi(numeric_body(),0,2);
		if (numeric_sign()=="-")
			return -out;
		return out;
	}

	int StringPlus::toInt_Hex() const
	{
		int out = stoi(numeric_body(),0,16);
		if (numeric_sign()=="-")
			return -out;
		return out;
	}

	int StringPlus::toInt_Decimal() const
	{
		return std::stoi(*this);
	}

	StringPlus StringPlus::make_hex(int n)
	{
		StringPlus out;
		int z = abs(n);
		if (z==0)
			out="0";
		while (z>0) {
			out.insert(0,HEX_SYMBOLS[z%16]);
			z/=16;
		}
		out.insert(0,"0x");
		if (n<0)
			out.insert(0,'-');
		return out;
	}

	StringPlus StringPlus::make_binary(int n)
	{
		StringPlus out;
		int z = abs(n);
		if (z==0)
			out="0";
		while (z>0) {
			out.insert(0,z%2==0?'0':'1');
			z/=2;
		}
		out.insert(0,"0b");
		if (n<0)
			out.insert(0,'-');
		return out;
	}

	StringPlus StringPlus::make_decimal(int n)
	{
		return std::to_string(n);
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
		while (true) {
			lastpos = find(val,lastpos+1);
			if (lastpos==std::string::npos)
				return out;
			else
				out.push_back(lastpos);
		}
	}

	std::vector<int> StringPlus::find_all_of(const StringPlus& val) const
	{
		std::vector<int> out;
		int lastpos = -1;
		while (true) {
			lastpos = find_first_of(val,lastpos+1);
			if (lastpos==std::string::npos)
				return out;
			else
				out.push_back(lastpos);
		}
	}

	std::vector<StringPlus> StringPlus::split(const StringPlus& delim) const
	{
		std::vector<StringPlus> out;
		if (delim.empty()) {
			out.push_back(*this);
			return out;
		}
		auto borders = find_all(delim);
		borders.insert(borders.begin(),-delim.size());
		borders.push_back(size());
		for (int i=1;i<borders.size();i++) {
			int len = borders[i]-borders[i-1]-delim.size();
			out.push_back(std::u32string::substr(borders[i-1]+delim.size(),len));
		}
		return out;
	}

	std::vector<StringPlus> StringPlus::split_all_of(const StringPlus& delim) const {
		std::vector<StringPlus> out;
		auto borders = find_all_of(delim);
		borders.insert(borders.begin(),-1);
		borders.push_back(size());
		for (int i=1;i<borders.size();i++) {
			int len = borders[i]-borders[i-1]-1;
			out.push_back(std::u32string::substr(borders[i-1]+1,len));
		}
		return out;
	}

	::std::ostream& operator<<(std::ostream& out, const StringPlus& str) {
		return out << str.toString();
	}

	StringPlus StringPlus::random(int len, const StringPlus& charset) {
		std::random_device rd;
		StringPlus out;
		out.resize(len);
		for (auto& c: out)
			c = charset[rd()%charset.size()];
		return out;
	}

	StringPlus StringPlus::toLowerCase() const {
		StringPlus out;
		for (int i=0;i<size();i++)
			out += tolower(at(i));
		return out;
	}

	StringPlus StringPlus::shrink() const {
		StringPlus out (*this);
		while (out.size()>0 && out.at(0)==' ')
			out.erase(0,1);
		while (out.size()>0 && out.back()==' ')
			out.pop_back();
		return out;
	}

	StringPlus StringPlus::normalize(const StringPlus& fname) const {
		StringPlus table = StringPlus::fromFile(fname);
		if (table == NO_CONTENT)
			return NO_CONTENT;
		StringPlus out = *this;
		std::vector<StringPlus> lines = table.split(ENDLINE);
		for (auto& line: lines) {
			std::vector<StringPlus> chars = line.split(" ");
			for (auto& c: chars) {
				std::vector<int> indexes = out.find_all(c);
				for (auto pos=indexes.rbegin(); pos!=indexes.rend(); pos++) {
					out.erase(*pos,c.size());
					out.insert(*pos,chars[0]);
				}
			}
		}
		return out;
	}

	StringPlus StringPlus::replace_all(const StringPlus& trigger, const StringPlus& subst) const {
		StringPlus out = *this;
		std::vector<int> triggers = out.find_all(trigger);
		for (auto pos=triggers.rbegin();pos!=triggers.rend();pos++)
			out.replace(*pos,trigger.size(),subst);
		return out;
	}

	StringPlus StringPlus::replace_all_of(const StringPlus& trigger, const StringPlus& subst) const {
		StringPlus out = *this;
		std::vector<int> triggers = out.find_all_of(trigger);
		for (auto pos=triggers.rbegin();pos!=triggers.rend();pos++)
			out.replace(*pos,1,subst);
		return out;
	}

	StringPlus StringPlus::substr(int start, size_t len) const {
		if (empty())
			return StringPlus();
		while (start<0)
			start+=size();
		return std::u32string::substr(start, len);
	}

	StringPlus StringPlus::interval(int start, int end) const {
		if (empty())
			return StringPlus();
		while (start<0)
			start+=size();
		while (end<0)
			end+=size();
		if (end<start)
			return StringPlus();
		return std::u32string::substr(start,end-start);
	}

	bool StringPlus::endsWith(const StringPlus& txt) const {
		return std::u32string::substr(size()-txt.size()) == txt;
	}

	bool StringPlus::beginsWith(const StringPlus& txt) const {
		return std::u32string::substr(0,txt.size()) == txt;
	}

	int StringPlus::count_all(const StringPlus& needle) const {
		return find_all(needle).size();
	}
	
	int StringPlus::count_all_of(const StringPlus& targets) const {
		return find_all_of(targets).size();
	}

	bool StringPlus::consists_of(const StringPlus& chars) const {
		for (auto& c: *this)
			if (chars.find(c)==npos)
				return false;
		return true;
	}
	
	bool StringPlus::nameCompare (const StringPlus& a, const StringPlus& b, const StringPlus& conversion_table) {
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
		return true;
	}

	std::istream& operator>>(std::istream& in, StringPlus& string) {
		std::string str;
		in >> str;
		string.assign(str);
		return in;
	}

	StringPlus StringPlus::fromFile (const StringPlus& fname) {
		std::fstream fs (fname.toString().c_str(),std::fstream::in | std::fstream::binary);
		if (!fs.good()) {
			fs.close();
			Exception e ("Datei konnte nicht gelesen werden: ");
			e+=fname;
			e.trace.push_back("StringPlus::fromFile()");
			throw e;
		}
		StringPlus out;
		while (fs.good()) {
			int c = fs.get();
			if (c!=EOF)
				out += c;
		}
		fs.close();
		return out;
	}

	void StringPlus::toFile(const StringPlus& content, const StringPlus& fname) {
		std::fstream fs(fname.toString().c_str(),std::fstream::out | std::fstream::binary);
		if (!fs.good()) {
			Exception e ("Datei konnte nicht geoeffnet werden: ");
			e+=fname;
			e.trace.push_back("StringPlus::toFile()");
			throw e;
		}
		std::string block = content.toString();
		fs.write(block.c_str(),block.size());
		if (!fs.good()) {
			Exception e ("Datei konnte nicht bearbeitet werden: ");
			e+=fname;
			e.trace.push_back("StringPlus::toFile()");
			throw e;
		}
		fs.close();
	}
}
