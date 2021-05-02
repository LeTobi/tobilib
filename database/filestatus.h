#ifndef TC_DATABASE_FILESTATUS_H
#define TC_DATABASE_FILESTATUS_H

#include "fileio.h"

namespace tobilib{
namespace database_detail{

class StatusFile: public File
{
public:
    StatusFile(Database*);

    void open();
    bool is_locked();
    bool lock();
    void unlock();

    bool get_fallback_enabled() const;
    void set_fallback_enabled(bool);
};

} // namespace database_detail
} // namespace tobilib

#endif