#define TC_DATABASE_INTERN

#include "tableaccess.h"
#include "database.h"
#include "../general/exception.hpp"
#include <cmath>

using namespace tobilib;
using namespace database_detail;

ListFile::ListFile(Database* db): CrashSafeFile(db) {};

void ListFile::open()
{
    if (!pre_init("ListFile::open()"))
        return;
    CrashSafeFile::open();
    filesize_t len = size();
    if (len < LINESIZE) {
        CrashSafeFile::extend(LINESIZE - len%LINESIZE);
        set_first_empty(0);
        set_last_empty(0);
        set_data_capacity(1);
    }
}

LineIndex ListFile::get_index(filesize_t position) const
{
    return position / LINESIZE;
}

LineIndex ListFile::get_first_empty() const
{
    return readAt<unsigned int>(0);
}

LineIndex ListFile::get_last_empty() const
{
    filesize_t where = 1*serial_size<unsigned int>();
    return readAt<unsigned int>(where);
}

LineIndex ListFile::get_data_capacity() const
{
    filesize_t where = 2*serial_size<unsigned int>();
    return readAt<unsigned int>(where);
}

LineIndex ListFile::get_next(LineIndex where) const
{
    filesize_t pos = where*LINESIZE;
    pos += 1*serial_size<unsigned int>();
    return readAt<unsigned int>(pos);
}

LineIndex ListFile::get_previous(LineIndex where) const
{
    filesize_t pos = where*LINESIZE;
    return readAt<unsigned int>(pos);
}

filesize_t ListFile::data_location(LineIndex where) const
{
    return where*LINESIZE+LINEHEAD;
}

void ListFile::set_first_empty(LineIndex what)
{
    writeAt(0,what);
}

void ListFile::set_last_empty(LineIndex what)
{
    filesize_t pos = 1*serial_size<unsigned int>();
    writeAt(pos,what);
}

void ListFile::set_data_capacity(LineIndex amount)
{
    filesize_t pos = 2*serial_size<unsigned int>();
    writeAt(pos,amount);
}

void ListFile::set_next(LineIndex where, LineIndex what)
{
    filesize_t pos = where*LINESIZE;
    pos += serial_size<unsigned int>();
    writeAt(pos,what);
}

void ListFile::set_previous(LineIndex where, LineIndex what)
{
    filesize_t pos = where*LINESIZE;
    writeAt(pos,what);
}

LineIndex ListFile::remove_empty()
{
    LineIndex first = get_first_empty();
    if (first==0)
    {
        extend_data_capacity();
        first = get_first_empty();
    }
    LineIndex next = get_next(first);
    set_first_empty(next);
    if (next==0)
        set_last_empty(0);
    else
        set_previous(next,0);
    return first;
}

void ListFile::append_empty(LineIndex where)
{
    LineIndex last = get_last_empty();
    if (last==0)
        set_first_empty(where);
    else
        set_next(last,where);
    set_previous(where,last);
    set_next(where,0);
    set_last_empty(where);
}

void ListFile::append_filled(LineIndex old_first, LineIndex new_first)
{
    set_next(new_first,old_first);
    set_previous(new_first,0);
    if (old_first!=0)
        set_previous(old_first,new_first);
}

LineIndex ListFile::remove_filled(LineIndex to_remove)
{
    LineIndex next = get_next(to_remove);
    LineIndex previous = get_previous(to_remove);
    if (next!=0)
        set_previous(next,previous);
    if (previous!=0)
        set_next(previous,next);
    return next;
}

void ListFile::extend_data_capacity()
{
    LineIndex filecapacity = size() / LINESIZE;
    LineIndex oldcapacity = get_data_capacity();
    if (filecapacity == oldcapacity)
    {
        filesize_t overhead = size() % LINESIZE;
        extend(LINESIZE - overhead);
    }
    set_data_capacity(oldcapacity+1);
    append_empty(oldcapacity);
}

LineIndex ListFile::emplace(LineIndex old_first)
{
    // Critical operation externally !
    LineIndex new_item = remove_empty();
    append_filled(old_first,new_item);
    return new_item;
}

LineIndex ListFile::erase(LineIndex to_remove)
{
    // Critical operation externally !
    LineIndex successor = remove_filled(to_remove);
    append_empty(to_remove);
    return successor;
}

const filesize_t ListFile::LINESIZE = 3*serial_size<unsigned int>();
const filesize_t ListFile::LINEHEAD = 2*serial_size<unsigned int>();

ClusterFile::ClusterFile(Database* db): CrashSafeFile(db)
{ }

void ClusterFile::open()
{
    if (!pre_init("ClusterFile::open()"))
        return;
    CrashSafeFile::open();
    filesize_t len = size();
    if (len<linesize())
    {
        CrashSafeFile::extend(linesize()-len%linesize());
        set_first_empty(0);
        set_last_empty(0);
        set_first_filled(0);
        set_last_filled(0);
        set_data_capacity(1);
    }
}

LineIndex ClusterFile::get_first_filled() const
{
    return readAt<unsigned int>(0);
}

LineIndex ClusterFile::get_first_empty() const
{
    return readAt<unsigned int>(2*serial_size<unsigned int>());
}

LineIndex ClusterFile::get_last_filled() const
{
    return readAt<unsigned int>(1*serial_size<unsigned int>());
}

LineIndex ClusterFile::get_last_empty() const
{
    return readAt<unsigned int>(3*serial_size<unsigned int>());
}

LineIndex ClusterFile::get_data_capacity() const
{
    return readAt<unsigned int>(4*serial_size<unsigned int>());
}

LineIndex ClusterFile::get_next(LineIndex where) const
{
    filesize_t pos = where*linesize();
    pos += serial_size<bool>() + serial_size<unsigned int>();
    return readAt<unsigned int>(pos);
}

LineIndex ClusterFile::get_previous(LineIndex where) const
{
    filesize_t pos = where*linesize();
    pos += serial_size<bool>();
    return readAt<unsigned int>(pos);
}

bool ClusterFile::get_occupied(LineIndex where) const
{
    filesize_t pos = where*linesize();
    return readAt<bool>(pos);
}

unsigned int ClusterFile::get_refcount(LineIndex where) const
{
    filesize_t pos = where*linesize();
    pos += serial_size<bool>() + 2*serial_size<unsigned int>();
    return readAt<unsigned int>(pos);
}

filesize_t ClusterFile::data_location(LineIndex where) const
{
    return linesize()*where + LINEHEAD;
}

void ClusterFile::set_first_filled(LineIndex what)
{
    writeAt(0,what);
}

void ClusterFile::set_first_empty(LineIndex what)
{
    writeAt(2*serial_size<unsigned int>(),what);
}

void ClusterFile::set_last_filled(LineIndex what)
{
    writeAt(1*serial_size<unsigned int>(),what);
}

void ClusterFile::set_last_empty(LineIndex what)
{
    writeAt(3*serial_size<unsigned int>(),what);
}

void ClusterFile::set_data_capacity(LineIndex amount)
{
    writeAt(4*serial_size<unsigned int>(),amount);
}

void ClusterFile::set_next(LineIndex where, LineIndex what)
{
    filesize_t pos = where*linesize();
    pos += serial_size<bool>() + serial_size<unsigned int>();
    writeAt(pos,what);
}

void ClusterFile::set_previous(LineIndex where, LineIndex what)
{
    filesize_t pos = where*linesize();
    pos += serial_size<bool>();
    writeAt(pos,what);
}

void ClusterFile::set_occupied(LineIndex where, bool what)
{
    filesize_t pos = where*linesize();
    writeAt(pos,what);
}

void ClusterFile::clear_refcount(LineIndex where)
{
    filesize_t pos = where*linesize();
    pos += serial_size<bool>()+2*serial_size<unsigned int>();
    writeAt<unsigned int>(pos,0);
}

void ClusterFile::set_refcount_add(LineIndex where, int add)
{
    unsigned int refcount = get_refcount(where);
    refcount+=add;
    filesize_t pos = where*linesize();
    pos += serial_size<bool>() + 2*serial_size<unsigned int>();
    writeAt(pos,refcount);
}

LineIndex ClusterFile::remove_empty()
{
    LineIndex first = get_first_empty();
    if (first == 0)
    {
        extend_data_capacity();
        first = get_first_empty();
    }
    LineIndex next = get_next(first);
    set_first_empty(next);
    if (next==0)
        set_last_empty(0);
    else
        set_previous(next,0);
    return first;
}

void ClusterFile::append_empty(LineIndex new_empty)
{
    LineIndex last = get_last_empty();
    if (last==0)
        set_first_empty(new_empty);
    else
        set_next(last,new_empty);
    
    set_previous(new_empty,last);
    set_next(new_empty,0);
    set_last_empty(new_empty);
}

void ClusterFile::append_filled(LineIndex new_filled)
{
    LineIndex last = get_last_filled();
    if (last==0)
        set_first_filled(new_filled);
    else
        set_next(last,new_filled);
    set_previous(new_filled,last);
    set_next(new_filled,0);
    set_last_filled(new_filled);
    clear_refcount(new_filled);
}

void ClusterFile::remove_filled(LineIndex to_remove)
{
    LineIndex next = get_next(to_remove);
    LineIndex previous = get_previous(to_remove);
    if (next==0)
        set_last_filled(previous);
    else
        set_previous(next,previous);
    if (previous==0)
        set_first_filled(next);
    else
        set_next(previous,next);
}

LineIndex ClusterFile::emplace()
{
    FlagRequest lock = database->begin_critical_operation();
        LineIndex new_filled = remove_empty();
        append_filled(new_filled);
        set_occupied(new_filled,true);
    database->end_critical_operation(lock);
    return new_filled;
}

void ClusterFile::erase(LineIndex to_remove)
{
    if (get_refcount(to_remove)>0)
        throw Exception("Removal of referenced Database-Element","Database::ClusterFile::erase");
    
    FlagRequest lock = database->begin_critical_operation();
        remove_filled(to_remove);
        append_empty(to_remove);
        set_occupied(to_remove,false);
    database->end_critical_operation(lock);
}

void ClusterFile::extend_data_capacity()
{
    LineIndex filecapacity = size() / linesize();
    LineIndex oldcapacity = get_data_capacity();
    if (filecapacity == oldcapacity)
    {
        filesize_t overhead = size() % linesize();
        extend(linesize() - overhead);
    }
    set_data_capacity(oldcapacity+1);
    append_empty(oldcapacity);
}

filesize_t ClusterFile::linesize() const
{
    filesize_t min_out = 5*serial_size<unsigned int>();
    filesize_t out = LINEHEAD + type.size;
    return std::max(out,min_out);
}

const filesize_t ClusterFile::LINEHEAD = 1*serial_size<bool>() + 3*serial_size<unsigned int>();