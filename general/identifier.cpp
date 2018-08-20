#include "identifier.h"
#include <random>

namespace tobilib
{
	ID::ID(ID_Keeper& keeper)
	{
		*this = keeper.create();
	}
	
	ID::ID()
	{
		name = ID_Name(new std::string());
	}
	
	void ID::dismiss()
	{
		name->clear();
	}
	
	bool ID::valid() const
	{
		return name->size()>0;
	}
	
	std::string ID::toString() const
	{
		return *name;
	}
	
	bool ID::operator==(const ID& other) const
	{
		return valid() && other.name.get() == name.get();
	}
	
	std::string ID_Keeper::randomstr()
	{
		std::random_device rd;
		std::string out;
		for (int i=0;i<namesize;i++)
		{
			out += symbols[rd()%symbols.size()];
		}
		return out;
	}
	
	ID ID_Keeper::create()
	{
		std::string n;
		for (int i=0;i<10;i++)
		{
			n = randomstr();
			bool exists = false;
			for (auto entry: entries)
			{
				if (entry->size()==0)
				{
					entry = ID_Name(new std::string(n));
					return ID(entry);
				}
				else if (*entry==n)
				{
					exists=true;
					break;
				}
			}
			if (exists)
				continue;
			entries.push_back(ID_Name(new std::string(n)));
			return ID(entries.back());
		}
		throw ID_error(std::string("Es konnte keine sichere, neue ID mit ")+std::to_string(namesize)+" Zeichen erstellt werden.");
	}
	
	ID ID_Keeper::parse(const std::string& n)
	{
		if (n.size()==0)
			return ID();
		for (auto entry: entries)
		{
			if (*entry==n)
				return ID(entry);
		}
		return ID();
	}
}