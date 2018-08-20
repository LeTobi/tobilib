#ifndef tc_gpio_h
#define tc_gpio_h

#include <fstream>
#include <exception>

namespace tobilib
{
	enum GPIO_value {
		LOW = 0,
		HIGH = 1
	};

	class GPIO_error: public std::exception
	{
	public:
		std::string message;
		template<class Text> GPIO_error(Text txt): message(txt) {}
		const char* what() const noexcept {return message.c_str();}
	};

	class GPO
	{
	public:
		GPO(int p): pin(p) {}
		GPO(int,GPIO_value);
		~GPO() {close();}
		
		void open();
		void set(GPIO_value);
		void close();
		int value();
		
	private:
		int pin;
		std::fstream valstream;
		GPIO_value lastset = GPIO_value::LOW;
	};
}

#ifdef TC_AS_HPP
	#include "gpio.cpp"
#endif

#endif