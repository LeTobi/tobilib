#include "../../database/database.h"
#include <iostream>

using namespace tobilib;

Logger cout = std::string("main: ");

void parser_read_test() {
	database::TypeParser parser;
	parser.path = "../testing/database/data_typeparser/";
	database::DatabaseInfo dbinfo;
	if (!parser.parse(dbinfo))
		return;
	for (auto& cluster: dbinfo.types)
	{
		cout << "  type: " << cluster.name << std::endl;
		for (auto& block: cluster.segmentation)
		{
			cout << "    " << block.amount << "x " << block.name << std::endl;
		}
	}
}

void parser_write_test() {
	database::TypeParser parser;
	parser.path = "../testing/database/data_typeparser/";
	database::DatabaseInfo dbinfo;
	if (!parser.parse(dbinfo))
		return;
	parser.path << "../data_copy/";
	if (!parser.write(dbinfo))
		return;
	cout << "Schreiben erfolgreich" << std::endl;
}

void access_create_test() {
	database::Access access;
	if (!access.init("../testing/database/data_copy/"))
		return;
	if (!access.open())
		return;
	cout << "Datenbank kopiert" << std::endl;
	database::Element list = access.list("Game");
	while (access.is_good() && list.size()<10)
		list.emplace_back();
	if (access.is_good())
		cout << "Games gefuellt mit 10 elementen" << std::endl;
}

void access_fill_test() {
	database::Access access;
	if (!access.init("../testing/database/data_test/"))
		return;
	if (!access.open())
		return;
	database::Element list = access.list("A");
	while (access.is_good() && list.size()<2)
		list.emplace_back();
	list[0]("name")="Das ist ein Name";
	list[0]("test")=true;
	list[0]("zahl")=123456789;
	list[0]("kommazahl")=1.23456;

	cout << (std::string)list[0]("name") << std::endl;
	cout << (bool)list[0]("test") << std::endl;
	cout << (int)list[0]("zahl") << std::endl;
	cout << (double)list[0]("kommazahl") << std::endl;
}

int main(int argc, const char** args) {
	if (argc<2) {
		cout << "Testfunktion angeben: " << std::endl;
		cout << "- parser_read" << std::endl;
		cout << "- parser_write" << std::endl;
		cout << "- access_create" << std::endl;
		cout << "- access_fill" << std::endl;
		return 0;
	}
	std::string fnc(args[1]);
	
	if (fnc=="parser_read") {
		parser_read_test();
	}
	else if (fnc=="parser_write") {
		parser_write_test();
	}
	else if (fnc=="access_create") {
		access_create_test();
	}
	else if (fnc=="access_fill") {
		access_fill_test();
	}
	else {
		cout << "unbekannter Test" << std::endl;
	}
	cout << "Programm beendet" << std::endl;
	return 0;
}