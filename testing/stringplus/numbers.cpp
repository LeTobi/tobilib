#include "../../stringplus/stringplus.h"
#include <iostream>

using namespace tobilib;

int main()
{
    std::cout << "Zahl eingeben: ";
    StringPlus n;
    std::cin >> n;
    std::cout << "numeric_sign: \"" << n.numeric_sign() << "\"" << std::endl;
    std::cout << "numeric_prefix: \"" << n.numeric_prefix() << "\"" << std::endl;
    std::cout << "numeric_body: \"" << n.numeric_body() << "\"" << std::endl;
    std::cout << "isInt: " << n.isInt() << std::endl;
    std::cout << "isDecimal: " << n.isDecimal() << std::endl;
    std::cout << "isHex: " << n.isHex() << std::endl;
    std::cout << "isBinary: " << n.isBinary() << std::endl;
    if (n.isInt())
    {   
        int i = n.toInt();
        std::cout << "toInt: " << i << std::endl;
        std::cout << "make_hex: " << StringPlus::make_hex(i) << std::endl;
        std::cout << "make_binary: " << StringPlus::make_binary(i) << std::endl;
        std::cout << "make_decimal: " << StringPlus::make_decimal(i) << std::endl;
    }
    return 0;
}