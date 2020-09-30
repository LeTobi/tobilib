#include "../../database/database.h"
#include <iostream>

using namespace tobilib;

Logger cout = std::string("main: ");

int main(int argc, const char** args) {
	Database db ("../testing/database/data_test/");
	db.init();
	db.open();

	auto b1 = db.list("B").emplace();

	for (int i=0;i<10;i++)
	{
		b1("liste").emplace() = db.list("A").emplace();
	}
	for (auto a: b1("liste"))
		a("name") = std::to_string(a->index()) + " xxx";

	while (db.list("B").begin()!=db.list("B").end())
		db.list("B").begin()->erase();

	for (auto x: db.list("A"))
		std::cout << (std::string)x("name") << std::endl;

	while (db.list("A").begin()!=db.list("A").end())
		db.list("A").begin()->erase();
	
	if (db.is_good())
		cout << "Erfolgreich" << std::endl;
	else
		cout << "Fehlgeschlagen" << std::endl;
	return 0;
}