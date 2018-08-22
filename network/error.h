#ifndef TC_NETWORK_ERROR_H
#define TC_NETWORK_ERROR_H

#include <exception>
#include <functional>

namespace tobilib
{
	class network_error: public std::exception
	{
	public:
		template<class strT> network_error(strT m): msg(m) {};
		std::string msg;
		const char* what() const noexcept {return msg.c_str();};
	};
}

#endif