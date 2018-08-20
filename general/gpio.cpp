#include "gpio.h"

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
			throw GPIO_error("Fehler beim Exportieren des Pins (Adminrechte?)");
		fs << pin;
		fs.close();
		fs.open((std::string("/sys/class/gpio/gpio")+std::to_string(pin)+"/direction").c_str(),std::fstream::out);
		if (!fs.good())
			throw GPIO_error("Der Pin konnte nicht aufgesetzt werden. (Adminrechte?)");
		fs << "out";
		fs.close();
		valstream.open((std::string("/sys/class/gpio/gpio")+std::to_string(pin)+"/value").c_str(),std::fstream::out);
		if (!valstream.good())
			throw GPIO_error("Es kann nicht auf den Pin zugegriffen werden. (Adminrechte?)");
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
				throw GPIO_error("Ungueltiger Wert");
		}
		valstream << msg;
		valstream.flush();
		if (!valstream.good())
			throw GPIO_error("Fehler beim Bearbeiten des Pins");
		lastset = val;
	}

	void GPO::close()
	{
		if (!valstream.is_open())
			return;
		valstream.close();
	}
}