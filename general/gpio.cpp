#include "gpio.h"
#include "exception.hpp"

namespace tobilib
{
	GPO::GPO(int p, GPIO_value val)
	{
		pin = p;
		open();
		set(val);
	}

	int GPO::value()
	{
		return lastset;
	}

	void GPO::open()
	{
		if (valstream.is_open())
			return;
		std::fstream fs;
		fs.open("/sys/class/gpio/export", std::fstream::out);
		if (!fs.good())
		{
			Exception err ("Fehler beim Exportieren des Pins (Adminrechte?)");
			err.trace.push_back("GPO::open()");
			throw err;
		}
		fs << pin;
		fs.close();
		fs.open((std::string("/sys/class/gpio/gpio")+std::to_string(pin)+"/direction").c_str(),std::fstream::out);
		if (!fs.good())
		{
			Exception err ("Der Pin konnte nicht aufgesetzt werden. (Adminrechte?)");
			err.trace.push_back("GPO::open()");
			throw err;
		}
		fs << "out";
		fs.close();
		valstream.open((std::string("/sys/class/gpio/gpio")+std::to_string(pin)+"/value").c_str(),std::fstream::out);
		if (!valstream.good())
		{
			Exception err ("Es kann nicht auf den Pin zugegriffen werden. (Adminrechte?)");
			err.trace.push_back("GPO::open()");
			throw err;
		}
	}

	void GPO::set(GPIO_value val)
	{
		std::string msg;
		switch (val)
		{
			case GPIO_value::HIGH:
				msg = "1";
				break;
			case GPIO_value::LOW:
				msg = "0";
				break;
			default:
				Exception err ("Ungueltiger Wert");
				err.trace.push_back("GPO::set()");
				throw err;
		}
		valstream << msg;
		valstream.flush();
		if (!valstream.good())
		{
			Exception err ("Fehler beim Bearbeiten des Pins");
			err.trace.push_back("GPO::set()");
			throw err;
		}
		lastset = val;
	}

	void GPO::close()
	{
		if (!valstream.is_open())
			return;
		valstream.close();
	}
}