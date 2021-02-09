#define TC_DATABASE_INTERN
#include "../database.h"
#include "../manipulation/export.h"

using namespace tobilib;

#include <iostream>
#include <fstream>

void printall(std::string source, std::string target = "")
{
    Database db (source);

    if (!db.init() || !db.open())
        return;

    database_tools::Result res;
    if (target.size()>0)
        res = database_tools::export_database(db,target);
    else
        res = database_tools::export_database(db);

    if (!res)
        std::cout << res.info << std::endl;
    else
        std::cout << "export erfolgreich" << std::endl;

    return;
}

int main(int argc, const char** args)
{
    if (!(argc==2 || argc==3))
    {
        std::cout << "Anwendung:\n> baseprinter [database] [output]" << std::endl;
        return 0;
    }

    std::string target;

    if (argc==3)
        target = args[2];
    
    printall(args[1], target);

    return 0;
}