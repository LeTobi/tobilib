﻿#ifndef stringplus_h
#define stringplus_h

#include <string>
#include <vector>
#include <exception>
#include <ostream>

namespace tobilib
{
	typedef char32_t CharPlus;
	
	class StringPlus : public std::u32string
	{
	public:

	// creation
		StringPlus ();
		StringPlus (const std::u32string&);
		StringPlus (const std::string&);
		StringPlus (const char*);
		StringPlus (const CharPlus&, int count = 1);
		
		void assign (const std::string&);
		void assign (const char *);
		void assign (const CharPlus&, int count = 1);
		StringPlus& insert (int, const StringPlus&);
		std::string toString () const;

	// basic parsing
		StringPlus numeric_sign() const;
		StringPlus numeric_prefix() const;
		StringPlus numeric_body() const;
		bool isInt() const;
		bool isDecimal() const;
		bool isHex() const;
		bool isBinary() const;
		int toInt() const;
		int toInt_Decimal() const;
		int toInt_Hex() const;
		int toInt_Binary() const;

		static StringPlus make_hex(int);
		static StringPlus make_binary(int);
		static StringPlus make_decimal(int);
		
	// modification
		int find(const StringPlus&, int start=0) const;
		int rfind(const StringPlus&, int start=-1) const;
		std::vector<int> find_all(const StringPlus&) const;
		std::vector<int> find_all_of(const StringPlus&) const;
		std::vector<StringPlus> split(const StringPlus&) const;
		std::vector<StringPlus> split_all_of(const StringPlus&) const;
		StringPlus toLowerCase() const;
		StringPlus shrink() const;
		StringPlus normalize(const StringPlus&) const;
		StringPlus replace_all(const StringPlus&, const StringPlus&) const;
		StringPlus replace_all_of(const StringPlus&, const StringPlus&) const;
		StringPlus substr(int, size_t len=npos) const;
		StringPlus interval(int, int) const;
		bool endsWith(const StringPlus&) const;
		bool beginsWith(const StringPlus&) const;
		int count_all(const StringPlus&) const;
		int count_all_of(const StringPlus&) const;
		bool consists_of(const StringPlus&) const;
		
		StringPlus& operator = (const std::string& val) {assign(val);return *this;};
		StringPlus& operator = (const char * val) {assign(val);return *this;};
		StringPlus& operator = (const CharPlus& val) {assign(val);return *this;};
		StringPlus operator + (const StringPlus& val) const {return StringPlus(val).insert(0,*this);}
		StringPlus operator + (const std::u32string& val) const {return StringPlus(val).insert(0,*this);};
		StringPlus operator + (const std::string& val) const {return StringPlus(val).insert(0,*this);};
		StringPlus operator + (const char * val) const {return StringPlus(val).insert(0,*this);};
		StringPlus& operator += (const StringPlus& val) {append(val);return *this;};
		StringPlus& operator += (const std::u32string& val) {append(val);return *this;};
		StringPlus& operator += (const std::string& val) {append(StringPlus(val));return *this;};
		StringPlus& operator += (const char * val) {append(StringPlus(val));return *this;};
		StringPlus& operator += (const CharPlus& val) {append(1,val); return *this;};
		friend std::istream& operator>>(std::istream&, StringPlus&);

		bool operator == (const char* val) const {return *this==StringPlus(val);};
		bool operator != (const char* val) const {return *this!=StringPlus(val);};
		
		operator std::string() const {return toString();};
		friend std::ostream& operator<<(std::ostream&, const StringPlus&);
		
	// defaults
		const static StringPlus NO_CONTENT;
		const static StringPlus ENDLINE;
		const static StringPlus DEFAULT_CHARSET;
		const static StringPlus HEX_SYMBOLS;
		
	// utility
		static StringPlus random(int len=10, const StringPlus& charset=DEFAULT_CHARSET);
		static bool nameCompare(const StringPlus&, const StringPlus&, const StringPlus& conversion_table = NO_CONTENT);
		static StringPlus fromFile(const StringPlus&);
		static void toFile(const StringPlus&, const StringPlus&);
	};
}

#ifdef TC_AS_HPP
	#include "stringplus.cpp"
#endif

#endif