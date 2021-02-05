#include "../../general/requestflag.h"
#include <random>
#include <thread>

using namespace tobilib;

struct User
{
    FlagRequest id;
    bool active = false;
};

int main()
{
    RequestFlag flag;
    User users [3];
    std::random_device rd;

    while (true)
    {
        for (User& user: users)
            std::cout << user.active?"1":"0";
        std::cout << " " << flag.is_requested()?"1":"0";

        while (!flag.events.empty())
        switch(flag.events.next())
        {
            case FlagEvent::requested:
                std::cout << " (requested)";
                break;
            case FlagEvent::dismissed:
                std::cout << " (dismissed)";
                break;
        }

        std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));

        int i = rd()%3;
        if (users[i].active)
            flag.dismiss(users[i].id);
        else
            users[i].id = flag.request();
        users[i].active = !users[i].active;
    }
    return 0;
}