#ifndef H2PROTOCOL_ERROR_H
#define H2PROTOCOL_ERROR_H

#include <exception>
#include <string>

namespace tobilib
{
	class h2parser_error: public std::exception
	{
	public:
		template<class strT> h2parser_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};
	
	class h2client_error: public std::exception
	{
	public:
		template<class strT> h2client_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};
	
	class protocol_error: public std::exception
	{
	public:
		template<class strT> protocoll_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};
}

#endif