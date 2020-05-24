#define private public
#include "../../database/database.h"
#include <iostream>

using namespace tobilib;

Logger cout = std::string("main: ");

int main(int argc, const char** args) {
	Database db;
	db.setPath("../testing/database/data_test/");
	db.init();
	db.open();

	db.list("B").emplace();
	Database::Element b1 = db.list("B")[1];

	std::cout << b1.reference_count() << " references" << std::endl;
	for (int i=0;i<10;i++) {
		b1("liste").emplace() = db.list("A").emplace();
	}
	std::cout << b1.reference_count() << " references" << std::endl;

	int count = 1;
	for (auto a: b1("liste"))
		a("zahl")=count++;
	for (auto a: db.list("A"))
		std::cout << (int)a("zahl") << std::endl;

	std::cout << db.listfile.get_first_empty() << " ";
	std::cout << db.listfile.get_last_empty() << std::endl;

	while (db.list("B").begin()!=db.list("B").end() && db.is_good())
		db.list("B").begin()->erase();
	while (db.list("A").begin()!=db.list("A").end() && db.is_good())
		db.list("A").begin()->erase();

	std::cout << db.listfile.get_first_empty() << " ";
	std::cout << db.listfile.get_last_empty() << std::endl;

	if (db.is_good())
		std::cout << "erfolgreich" << std::endl;
	else 
		std::cout << "fehlgeschlagen" << std::endl;
	return 0;
}