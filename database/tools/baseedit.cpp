#include <iostream>
#include "../database.h"
#include "../manipulation/commands.h"

using namespace tobilib;
using namespace database_tools;

void run(std::string path)
{
    Database db (path);

    if (!db.init() || !db.open())
        return;

    while (true)
    {
        std::cout << "> " << std::flush;
        std::string input;
        std::getline(std::cin,input);
        if (input == "quit")
            return;
        std::string result = command(db,input);
        std::cout << result << "\n";

        if (!db.is_good())
            std::cout << "Fehler in der Datenbank" << std::endl;
    }
}

int main(int argc, const char** args)
{
    if (argc<2)
    {
        std::cout << "Anwendung: \n> baseedit [database]" << std::endl;
        return 0;
    }

    run(args[1]);
    return 0;
}