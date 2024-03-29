#include "filestatus.h"

using namespace tobilib;
using namespace database_detail;

StatusFile::StatusFile(Database* db): File(db)
{ }

void StatusFile::open()
{
    File::open();
    if (size()==0)
        writeAt<bool>(0,false);
}

bool StatusFile::is_locked()
{
    return File::is_locked(0,1);
}

bool StatusFile::lock()
{
    return File::lock(0,1);
}

void StatusFile::unlock()
{
    File::unlock(0,1);
}

bool StatusFile::get_fallback_enabled() const
{
    return readAt<bool>(0);
}

void StatusFile::set_fallback_enabled(bool val)
{
    writeAt<bool>(0,val);
    save();
}