#define TC_DATABASE_INTERN
#include "database.h"
#include <cmath>

using namespace tobilib;
using namespace database_detail;

template<class PrimT>
std::streampos database_detail::serial_size()
{
    return sizeof(PrimT);
}

template<bool>
std::streampos database_detail::serial_size()
{
    return sizeof(char);
}

template std::streampos database_detail::serial_size<int>();
template std::streampos database_detail::serial_size<unsigned int>();
template std::streampos database_detail::serial_size<double>();
template std::streampos database_detail::serial_size<char>();

template<class PrimT>
bool serial_print(std::fstream& fs, PrimT out)
{
    char* data = reinterpret_cast<char*>(&out);
    for (int i=0;i<sizeof(PrimT);i++) {
        fs.put(data[i]);
    }
    return fs.good();
}

template<bool>
bool serial_print(std::fstream& fs, bool out)
{
    fs << out?'1':'0';
    return fs.good();
}

template<class PrimT>
bool serial_read(std::fstream& fs, PrimT& in) {
    char* data = reinterpret_cast<char*>(&in);
    for (int i=0;i<sizeof(PrimT);i++) {
        data[i] = fs.get();
    }
    return fs.good();
}

template<bool>
bool serial_read(std::fstream& fs, bool& in) {
    in = fs.get()=='1';
    return fs.good();
}

File::File(Database* db): Component(db)
{ }

void File::open()
{
    if (!pre_good())
        return;
    fs.close();
    fs.open(name.fullName(),fs.in|fs.out);
    if (fs.good())
        return;
    fs.close();
    fs.open(name.fullName(),fs.out);
    if (!fs.good()) {
        database->status = Database::Status::error;
        database->log << name.fullName() << " konnte nicht geoeffnet werden" << std::endl;
        return;
    }
    database->log << name.fullName() << " wurde neu erstellt" << std::endl;
    fs.close();
    fs.open(name.fullName(),fs.in|fs.out);
    if (!fs.good())
    {
        database->status = Database::Status::error;
        database->log << name.fullName() << " kann nicht gelesen werden" << std::endl;
        return;
    }
}

void File::close()
{
    fs.close();
}

std::streampos File::size() const
{
    if (!pre_good())
        return 0;
    fs.seekg(0,fs.end);
    std::streampos len = fs.tellg();
    if (!fs.good()) {
        database->status = Database::Status::error;
        database->log << "Fehler bei Ermittlung der Dateigroesse: (" << name.fileOnly() << ")" << std::endl;
        return 0;
    }
    return len;
}

template<class PrimT>
PrimT File::readAt(std::streampos where) const
{
    if (!pre_good())
        return 0;
    fs.seekg(where);
    PrimT out;
    if (!serial_read(fs,out)) {
        database->status = Database::Status::error;
        database->log << "Lesefehler an Position " << fs.tellg() << " (" << name.fileOnly() << ")" << std::endl;
        return 0;
    }
    return out;
}

template int          File::readAt(std::streampos) const;
template unsigned int File::readAt(std::streampos) const;
template double       File::readAt(std::streampos) const;
template char         File::readAt(std::streampos) const;
template bool         File::readAt(std::streampos) const;

template <class PrimT>
void File::writeAt(std::streampos where, PrimT what)
{
    if (!pre_good())
        return;
    fs.seekp(where);
    if (!serial_print(fs,what)) {
        database->status = Database::Status::error;
        database->log << "Schreibfehler an Position " << fs.tellp() << " (" << name.fileOnly() << ")" << std::endl;
    }
}

template void File::writeAt(std::streampos,int);
template void File::writeAt(std::streampos,unsigned int);
template void File::writeAt(std::streampos,double);
template void File::writeAt(std::streampos,char);
template void File::writeAt(std::streampos,bool);

ListFile::ListFile(Database* db): File(db) {};

void ListFile::open()
{
    if (!pre_init())
        return;
    File::open();
    std::streampos len = size();
    if (len==0) {
        fs << std::string(LINESIZE-2*serial_size<char>(),0);
        fs << "\r\n";
        set_first_empty(0);
        set_last_empty(0);
    }
}

ListFile::LineIndex ListFile::get_index(std::streampos position) const
{
    return position / LINESIZE;
}

ListFile::LineIndex ListFile::capacity() const
{
    if (!pre_open())
        return 0;
    return size() / LINESIZE - 1;
}

ListFile::LineIndex ListFile::get_first_empty() const
{
    return readAt<unsigned int>(0);
}

ListFile::LineIndex ListFile::get_last_empty() const
{
    std::streampos where = 1*serial_size<unsigned int>();
    return readAt<unsigned int>(where);
}

ListFile::LineIndex ListFile::get_next(LineIndex where) const
{
    std::streampos pos = where*LINESIZE;
    pos += 1*serial_size<unsigned int>();
    return readAt<unsigned int>(pos);
}

ListFile::LineIndex ListFile::get_previous(LineIndex where) const
{
    std::streampos pos = where*LINESIZE;
    return readAt<unsigned int>(pos);
}

std::streampos ListFile::data_location(LineIndex where) const
{
    return where*LINESIZE+LINEHEAD;
}

void ListFile::set_first_empty(LineIndex what)
{
    writeAt(0,what);
}

void ListFile::set_last_empty(LineIndex what)
{
    std::streampos pos = serial_size<unsigned int>();
    writeAt(pos,what);
}

void ListFile::set_next(LineIndex where, LineIndex what)
{
    std::streampos pos = where*LINESIZE;
    pos += serial_size<unsigned int>();
    writeAt(pos,what);
}

void ListFile::set_previous(LineIndex where, LineIndex what)
{
    std::streampos pos = where*LINESIZE;
    writeAt(pos,what);
}

ListFile::LineIndex ListFile::extend()
{
    if (!pre_open())
        return 0;
    fs.seekp(0,fs.end);
    std::streampos new_space = fs.tellp();
    fs << std::string(LINESIZE-2*serial_size<char>(),0);
    fs << "\r\n";
    fs.flush();
    if (!fs.good()) {
        database->status = Database::Status::error;
        database->log << "Fehler beim Erweitern der Listendatei" << std::endl;
        return 0;
    }
    return new_space/LINESIZE;
}

ListFile::LineIndex ListFile::remove_empty()
{
    LineIndex first = get_first_empty();
    if (first==0)
        return extend();
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

ListFile::LineIndex ListFile::remove_filled(LineIndex to_remove)
{
    LineIndex next = get_next(to_remove);
    LineIndex previous = get_previous(to_remove);
    if (next!=0)
        set_previous(next,previous);
    if (previous!=0)
        set_next(previous,next);
    return next;
}

ListFile::LineIndex ListFile::emplace(LineIndex old_first)
{
    LineIndex new_item = remove_empty();
    append_filled(old_first,new_item);
    return new_item;
}

ListFile::LineIndex ListFile::erase(LineIndex to_remove)
{
    LineIndex successor = remove_filled(to_remove);
    append_empty(to_remove);
    return successor;
}

const std::streampos ListFile::LINESIZE = 3*serial_size<unsigned int>() + 2;
const std::streampos ListFile::LINEHEAD = 2*serial_size<unsigned int>();

ClusterFile::ClusterFile(Database* db): File(db)
{ }

void ClusterFile::open()
{
    if (!pre_init())
        return;
    File::open();
    std::streampos len = size();
    if (len==0)
    {
        fs << std::string(linesize()-2*serial_size<char>(),0);
        fs << "\r\n";
        set_first_empty(0);
        set_last_empty(0);
        set_first_filled(0);
        set_last_filled(0);
    }
}

ClusterFile::LineIndex ClusterFile::get_first_filled() const
{
    return readAt<unsigned int>(0);
}

ClusterFile::LineIndex ClusterFile::get_first_empty() const
{
    return readAt<unsigned int>(2*serial_size<unsigned int>());
}

ClusterFile::LineIndex ClusterFile::get_last_filled() const
{
    return readAt<unsigned int>(1*serial_size<unsigned int>());
}

ClusterFile::LineIndex ClusterFile::get_last_empty() const
{
    return readAt<unsigned int>(3*serial_size<unsigned int>());
}

ClusterFile::LineIndex ClusterFile::get_next(LineIndex where) const
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>() + serial_size<unsigned int>();
    return readAt<unsigned int>(pos);
}

ClusterFile::LineIndex ClusterFile::get_previous(LineIndex where) const
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>();
    return readAt<unsigned int>(pos);
}

bool ClusterFile::get_occupied(LineIndex where) const
{
    std::streampos pos = where*linesize();
    return readAt<bool>(pos);
}

unsigned int ClusterFile::get_refcount(LineIndex where) const
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>() + 2*serial_size<unsigned int>();
    return readAt<unsigned int>(pos);
}

ClusterFile::LineIndex ClusterFile::capacity() const
{
    if (!pre_open())
        return 0;
    return size() / linesize() - 1;
}

std::streampos ClusterFile::data_location(LineIndex where) const
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

void ClusterFile::set_next(LineIndex where, LineIndex what)
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>() + serial_size<unsigned int>();
    writeAt(pos,what);
}

void ClusterFile::set_previous(LineIndex where, LineIndex what)
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>();
    writeAt(pos,what);
}

void ClusterFile::set_occupied(LineIndex where, bool what)
{
    std::streampos pos = where*linesize();
    writeAt(pos,what);
}

void ClusterFile::clear_refcount(LineIndex where)
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>()+2*serial_size<unsigned int>();
    writeAt<unsigned int>(pos,0);
}

void ClusterFile::set_refcount_add(LineIndex where, int add)
{
    unsigned int refcount = get_refcount(where);
    refcount+=add;
    std::streampos pos = where*linesize();
    pos += serial_size<bool>() + 2*serial_size<unsigned int>();
    writeAt(pos,refcount);
}

ClusterFile::LineIndex ClusterFile::extend()
{
    if (!pre_open())
        return size();
    fs.seekp(0,fs.end);
    LineIndex new_space = fs.tellp() / linesize();
    fs << std::string(linesize()-2*serial_size<char>(), 0);
    fs << "\r\n";
    fs.flush();
    if (!fs.good()) {
        database->status = Database::Status::error;
        database->log << "Datei konnte nicht erweitert werden: " << name.fileOnly() << std::endl;
    }
    return new_space;
}

ClusterFile::LineIndex ClusterFile::remove_empty()
{
    LineIndex first = get_first_empty();
    if (first == 0)
        return extend();
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

ClusterFile::LineIndex ClusterFile::emplace()
{
    LineIndex new_filled = remove_empty();
    append_filled(new_filled);
    set_occupied(new_filled,true);
    return new_filled;
}

void ClusterFile::erase(LineIndex to_remove)
{
    if (get_refcount(to_remove)>0)
        throw Exception("Removal of referenced Database-Element","Database::ClusterFile::erase");
    remove_filled(to_remove);
    append_empty(to_remove);
    set_occupied(to_remove,false);
}

std::streampos ClusterFile::linesize() const
{
    std::streampos min_out = LINEHEAD + 2l;
    std::streampos out = LINEHEAD + type.size + 2l;
    return std::max(out,min_out);
}

const std::streampos ClusterFile::LINEHEAD = 1*serial_size<bool>() + 3*serial_size<unsigned int>();