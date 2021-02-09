#include <iostream>
#include "../../stringplus/filename.h"

using namespace tobilib;

int main() {

    FileName file = "../../Datei.exe";
    FileName path = "Das/ist/ein/pfad/";

    std::cout << path << " + " << file << " = " << path+file << std::endl;

    path = "testpath/";
    file = "fileonly.txt";

    std::cout << path << " + " << file << " = " << path+file << std::endl;

    path = "onlyfile";
    file = ".ext";

    std::cout << path << " + " << file << " = " << path+file << std::endl;

    return 0;
}