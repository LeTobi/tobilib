#include "../../general/exception.hpp"

using namespace tobilib;

int main()
{
    Logger system ("System: ");
    Logger device ("device: ");
    Logger process ("process: ");
    device.parent = &system;
    process.parent = &device;
    process << "finished" << std::endl;
    return 0;
}