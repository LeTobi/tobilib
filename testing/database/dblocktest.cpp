#include "../../database/database.h"
#include <iostream>

using namespace tobilib;

int main()
{
    std::cout << "Teste file locks:" << std::endl;

    Database db1 ("../testing/database/data_test/");
    Database db2 ("../testing/database/data_test/");

    db1.init();
    db2.init();
    db1.open();

    if (!db1.is_good() || !db2.is_good())
    {
        std::cout << "Fehler bei setup" << std::endl;
        return 0;
    }

    db2.open();

    if (db2.is_good())
    {
        std::cout << "Lock hat nicht funktioniert" << std::endl;
        return 0;
    }
    
    std::cout << "Der Test war erfolgreich!" << std::endl;

    db1.close();
    db2.close();
}