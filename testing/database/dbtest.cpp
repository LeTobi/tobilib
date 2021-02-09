#include "../../database/database.h"
#include <iostream>
#include <sstream>

using namespace tobilib;

struct GlobalData
{
	Database base;
	const Database::Cluster nullCluster;

	GlobalData(): nullCluster(), base("../testing/database/data_test/")
	{ }
};

Logger cout = std::string("main: ");
GlobalData data;

bool init()
{
	data.base.init();
	data.base.open();

	if (!data.base.is_good())
	{
		std::cout << "error with init" << std::endl;
		return false;
	}
	return true;
}

bool reference_test()
{
	auto target1 = data.base.list("AdvancedClass").emplace();
	auto target2 = data.base.list("SimpleClass").emplace();
	auto item1 = data.base.list("SimpleClass").emplace();
	auto item2 = data.base.list("AdvancedClass").emplace();

	item1["memberPointer"].set( target1 );
	item2["memberList"].emplace().set( target2 );

	if (!data.base.is_good())
	{
		std::cout << "error with setup" << std::endl;
		return false;
	}

	try {
		target1.erase();
		std::cout << "expected error when erasing target 1" << std::endl;
		return false;
	}
	catch (...)
	{ 
		std::cout << "ok1" << std::endl;
	}

	try {
		target2.erase();
		std::cout << "expected error when erasing target 2" << std::endl;
		return false;
	}
	catch (...)
	{
		std::cout << "ok2" << std::endl;
	}

	target1.clear_references();
	target2.clear_references();

	/*
	if (item1("memberPointer")==data.nullCluster || *item2("memberList").begin() == data.nullCluster)
	{
		std::cout << "unwanted sideeffect of clear_references" << std::endl;
		return false;
	}
	*/

	item1.clear_references();
	item2.clear_references();

	if (!data.base.is_good())
	{
		std::cout << "error with clear_reference()" << std::endl;
		return false;
	}

	/*
	if (item1("memberPointer")!=data.nullCluster || *item2("memberList").begin()!=data.nullCluster)
	{
		std::cout << "expected null references" << std::endl;
		return false;
	}
	*/

	target1.erase();
	target2.erase();
	item1.erase();
	item2.erase();

	if (!data.base.is_good())
	{
		std::cout << "error with cleanup" << std::endl;
		return false;
	}

	return true;
}

bool list_test()
{
	auto simpleList = data.base.list("SimpleClass");
	auto advancedList = data.base.list("AdvancedClass");
	auto advanced = advancedList.emplace();

	for (int i=0;i<10;i++)
	{
		advanced["memberList"].emplace().set(simpleList.emplace());
	}
	for (auto simple: advanced["memberList"])
		simple["memberString"].set(
			std::string("this item is at ") + std::to_string(simple->index())
			);

	while (advancedList.begin()!=advancedList.end())
		advancedList.begin()->erase();

	for (auto x: simpleList)
		std::cout << x["memberString"].get<std::string>() << std::endl;

	while (simpleList.begin()!=simpleList.end())
		simpleList.begin()->erase();
	
	if (data.base.is_good())
	{
		return true;
	}
	else
	{
		cout << "Fehlgeschlagen" << std::endl;
		return false;
	}
}

bool reopen_test()
{
	unsigned int index = data.base.list("AdvancedClass").emplace().index();
	data.base.list("AdvancedClass")[index]["memberIntArray"][1].set(50);
	data.base.close();

	data.base.open();

	if (!data.base.is_good())
		return false;

	int result = data.base.list("AdvancedClass")[index]["memberIntArray"][1].get<int>();
	if (result == 50)
	{
		cout << "Reopen erfolgreich" << std::endl;
	}
	else
	{
		cout << "Falscher Eintrag nach reopen" << std::endl;
		return false;
	}
	data.base.list("AdvancedClass")[index].erase();
	return true;
}

int main(int argc, const char** args) {

	Database::Cluster nullItem;
	nullItem.erase();

	std::cout << "init" << std::endl;
	if (!init())
		return 0;
	//*
	std::cout << "reference test" << std::endl;
	if (!reference_test())
		return 0;
	//*/
	std::cout << "list test" << std::endl;
	if (!list_test())
		return 0;

	std::cout << "reopen test" << std::endl;
	if (!reopen_test())
		return 0;

	std::cout << "Alle Tests erfolgreich" << std::endl;
	return 0;
}