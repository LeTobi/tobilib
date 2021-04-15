#include "../../general/exception.hpp"

using namespace tobilib;

int main()
{
    Logger system ("System: ");
    Logger device ("device: ");
    Logger process ("process: ");
    device.parent = &system;
    process.parent = &device;
    process << "this is without time" << std::endl;
    system.print_time = true;
    process.print_time = true;
    process << "this should be with time" << std::endl;
    return 0;
}