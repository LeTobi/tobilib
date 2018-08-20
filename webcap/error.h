#ifndef TC_WEBCAP_ERROR_H
#define TC_WEBCAP_ERROR_H

#include <exception>
#include <string>

namespace tobilib
{
	class buffer_error: public std::exception
	{
	public:
		template<class strT> buffer_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};
	
	class jpeg_error: public std::exception
	{
	public:
		template<class strT> jpeg_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};
	
	class capture_error: public std::exception
	{
	public:
		template<class strT> capture_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};
}

#endif