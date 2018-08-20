#ifndef TC_IDENTIFIER_H
#define TC_IDENTIFIER_H

#include <string>
#include <vector>
#include <memory>

namespace tobilib
{
	typedef std::shared_ptr<std::string> ID_Name;
	
	class ID_Keeper;
	
	class ID
	{
	private:
		ID_Name name;
		
	public:
		ID (const ID_Name& n): name(n) {};
		ID (ID_Keeper&);
		ID ();
		
		void dismiss();
		bool valid() const;
		std::string toString() const;
		bool operator==(const ID&) const;
	};
	
	class ID_error: public std::exception
	{
	public:
		std::string message;
		template<class Text> ID_error(Text txt): message(txt) {}
		const char* what() const noexcept {return message.c_str();}
	};
	
	class ID_Keeper
	{
	private:
		std::vector<ID_Name> entries;
		
		const std::string symbols = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		std::string randomstr();
		const int namesize;
		
	public:
		ID_Keeper(int s=10): namesize(s) {};
		
		ID_Keeper& operator=(const ID_Keeper&) = delete;
		ID_Keeper(const ID_Keeper&) = delete;
		
		ID create();
		ID parse(const std::string&);
	};
	
	
}

#ifdef TC_AS_HPP
	#include "identifier.cpp"
#endif

#endif