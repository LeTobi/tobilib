#include "../../database/database.h"
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <thread>
#include <random>

using namespace tobilib;

Logger worklog = std::string("work-process: ");
Logger mainlog = std::string("main-process: ");
const std::string DBPATH = "../testing/database/data_test/";

void hardwork()
{
    worklog << "initialize..." << std::endl;
    Database db(DBPATH);
    db.log.parent = &worklog;
    if (!db.init() || !db.open())
        return;
    std::random_device rd;
    Database::Cluster maincluster;
    Database::ClusterList list = db.list("AdvancedClass");
    if (list.begin() == list.end())
        maincluster = list.emplace();
    else
        maincluster = *list.begin();
    Database::Member container = maincluster["memberList"];
    container.clear_references();
    Database::ClusterList freeItems = db.list("SimpleClass");
    for (Database::clusterIterator it=freeItems.begin();it!=freeItems.end();++it)
    {
        while (it!=freeItems.end() && it->reference_count()==0)
            (it++)->erase();
    }
    int size = 0;
    worklog << "running" << std::endl;

    while (true)
    {
        if (size==0 || rd()%100>45)
        {
            //add
            container.emplace().set( db.list("SimpleClass").emplace() );
            size++;
        }
        else
        {
            //remove
            int i = rd()%size;
            Database::MemberIterator it = container.begin();
            while(i-->0)
                ++it;
            Database::Cluster waist = **it;
            container.erase(it);
            waist.erase();
            size--;
        }
    }
}

void inspect()
{
    Database db(DBPATH);
    db.log.parent = &worklog;
    if (!db.init() || !db.open())
        return;
    
    unsigned int count = 0;
    for (Database::Cluster simple: db.list("SimpleClass"))
        count++;
    mainlog << count << " simple classes found" << std::endl;

    count = 0;
    for (Database::Cluster advanced: db.list("AdvancedClass"))
        count++;
    mainlog << count << " advanced classes found" << std::endl;

    count = 0;
    Database::Cluster maincluster = *db.list("AdvancedClass").begin();
    for (Database::Member m: maincluster["memberList"])
        count++;
    mainlog << count << " items found in list" << std::endl;

    db.close();
}

int main()
{
    std::cout << "*Crashtest:" << std::endl;
    std::cout << "*  Die Datenbank wird in einem work-prozess stark beansprucht und wird auf kommando unterbrochen." << std::endl
        << "*  Die Datenbank wird darauf erneut im Main prozess geoeffnet." << std::endl;
    
    while (true)
    {
        mainlog << "Taste Druecken um zu starten:" << std::endl;
        fflush(stdin);
        std::cin.get();

        int child = fork();
        if (child==0)
        {
            hardwork();
            return 0;
        }

        mainlog << "Taste Druecken um zu unterbrechen..." << std::endl;
        fflush(stdin);
        std::cin.get();
        kill(child,SIGKILL);
        waitpid(child,nullptr,0);
        inspect();
    }
    return 0;
}