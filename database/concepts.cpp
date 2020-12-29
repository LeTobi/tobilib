#define TC_DATABASE_INTERN
#include "database.h"

using namespace tobilib;
using namespace database_detail;

Component::Component(Database* db): database(db), nullflag(false)
{ }

Component::Component(): database(nullptr), nullflag(true)
{ }

bool Component::is_null() const
{
    return nullflag;
}

bool Component::pre_good() const
{
    if (nullflag)
        return false;
    return database->status != Database::Status::error;
}

bool Component::pre_init() const
{
    if (!pre_good())
        return false;
    if (database->status == Database::Status::empty) {
        database->status = Database::Status::error;
        database->log << "Anwendungsfehler: erwarte initialisierte datenbank" << std::endl;
        return false;
    }
    return true;
}

bool Component::pre_open() const
{
    if (!pre_init())
        return false;
    if (database->status!=Database::Status::open) {
        database->status = Database::Status::error;
        database->log << "Anwendungsfehler: erwarte offene datanbank" << std::endl;
        return false;
    }
    return true;
}