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

bool Component::pre_init(const std::string& trace) const
{
    if (!pre_good())
        return false;
    if (database->status == Database::Status::empty)
    {
        Exception ex("Precondition verletzt: erwarte initialisierte datenbank","Database_detail::Component::pre_init()");
        ex.trace.push_back(trace);
        throw ex;
    }
    return true;
}

bool Component::pre_open(const std::string& trace) const
{
    if (!pre_init(trace))
        return false;
    if (database->status!=Database::Status::open)
    {
        Exception ex("Precondition verletzt: erwarte offene Datenbank","Database_detail::Component::pre_open()");
        ex.trace.push_back(trace);
        throw ex;
    }
    return true;
}