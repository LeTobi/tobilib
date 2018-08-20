#ifndef TC_ENCODING_ERROR_H
#define TC_ENCODING_ERROR_H

#include <exception>

namespace tobilib
{
	class encoding_error: public std::exception
	{
	public:
		template<class strT> encoding_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};
}

#endif