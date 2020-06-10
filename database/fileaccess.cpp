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

Member ListFile::create_member(const Member& parent, unsigned int index) const
{
    if (index==0)
        return Member();
    std::streampos pos = index*LINESIZE + LINEHEAD;
    MemberType mt = parent.type;
    mt.blockType = BlockType::t_ptr;
    return Member(
        const_cast<Database*>(database),
        mt,
        const_cast<ListFile*>(this),
        pos);
}

unsigned int ListFile::get_index(const Member& mem) const
{
    return mem.position / LINESIZE;
}

unsigned int ListFile::get_first_empty() const
{
    return readAt<unsigned int>(0);
}

unsigned int ListFile::get_last_empty() const
{
    std::streampos where = 1*serial_size<unsigned int>();
    return readAt<unsigned int>(where);
}

unsigned int ListFile::get_next(unsigned int where) const
{
    std::streampos pos = where*LINESIZE;
    pos += 1*serial_size<unsigned int>();
    return readAt<unsigned int>(pos);
}

unsigned int ListFile::get_previous(unsigned where) const
{
    std::streampos pos = where*LINESIZE;
    return readAt<unsigned int>(pos);
}

unsigned int ListFile::get_first_filled(const Member& parent) const
{
    return parent.fs->readAt<unsigned int>(parent.position);
}

void ListFile::set_first_empty(unsigned int what)
{
    writeAt(0,what);
}

void ListFile::set_last_empty(unsigned int what)
{
    std::streampos pos = serial_size<unsigned int>();
    writeAt(pos,what);
}

void ListFile::set_next(unsigned int where, unsigned int what)
{
    std::streampos pos = where*LINESIZE;
    pos += serial_size<unsigned int>();
    writeAt(pos,what);
}

void ListFile::set_previous(unsigned int where, unsigned int what)
{
    std::streampos pos = where*LINESIZE;
    writeAt(pos,what);
}

void ListFile::set_first_filled(const Member& parent, unsigned int what)
{
    parent.fs->writeAt(parent.position,what);
}

unsigned int ListFile::extend()
{
    if (!pre_open())
        return 0;
    fs.seekp(0,fs.end);
    std::streampos new_space = fs.tellp();
    fs << std::string(LINESIZE-2*serial_size<char>(),0);
    fs << "\r\n";
    if (!fs.good()) {
        database->status = Database::Status::error;
        database->log << "Fehler beim Erweitern der Listendatei" << std::endl;
        return 0;
    }
    return new_space/LINESIZE;
}

unsigned int ListFile::remove_empty()
{
    unsigned int first = get_first_empty();
    if (first==0)
        return extend();
    unsigned int next = get_next(first);
    set_first_empty(next);
    if (next==0)
        set_last_empty(0);
    return first;
}

void ListFile::append_empty(unsigned int where)
{
    unsigned int last = get_last_empty();
    if (last==0)
        set_first_empty(where);
    else
        set_next(last,where);
    set_last_empty(where);
}

void ListFile::append_filled(const Member& parent, unsigned int new_item)
{
    unsigned int first = get_first_filled(parent);
    set_next(new_item,first);
    set_previous(new_item,0);
    if (first!=0)
        set_previous(first,new_item);
    else
        set_next(new_item,0);
    set_first_filled(parent,new_item);
}

void ListFile::remove_filled(const Member& parent, unsigned int to_remove)
{
    unsigned int next = get_next(to_remove);
    unsigned int previous = get_previous(to_remove);
    if (next!=0)
        set_previous(next,previous);
    if (previous!=0)
        set_next(previous,next);
    else
        set_first_filled(parent,next);
}

Member ListFile::emplace(const Member& parent)
{
    unsigned int new_item = remove_empty();
    append_filled(parent,new_item);
    return create_member(parent,new_item);
}

void ListFile::erase(const Member& parent, const Member& to_remove)
{
    unsigned int remove_index = get_index(to_remove);
    remove_filled(parent,remove_index);
    append_empty(remove_index);
}

Member ListFile::begin(const Member& parent)
{
    return create_member(parent, get_first_filled(parent));
}

Member ListFile::next(const Member& parent, const Member& current)
{
    return create_member( parent,
        get_next(
            get_index(current)
        )
    );
}

const Member ListFile::begin(const Member& parent) const
{
    return const_cast<ListFile*>(this)->begin(parent);
}

const Member ListFile::next(const Member& parent, const Member& current) const
{
    return const_cast<ListFile*>(this)->next(parent,current);
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

Cluster ClusterFile::create_cluster(unsigned int index) const
{
    if (index==0)
        return Cluster();
    if (index >= size() / linesize())
        return Cluster();
    if (!get_occupied(index))
        return Cluster();
    std::streampos pos = index*linesize()+LINEHEAD;
    return Cluster(
        const_cast<Database*>(database),
        const_cast<ClusterFile*>(this),
        pos);
}

unsigned int ClusterFile::get_index(const Cluster& cluster) const
{
    return cluster.position / linesize();
}

unsigned int ClusterFile::get_first_filled() const
{
    return readAt<unsigned int>(0);
}

unsigned int ClusterFile::get_first_empty() const
{
    return readAt<unsigned int>(2*serial_size<unsigned int>());
}

unsigned int ClusterFile::get_last_filled() const
{
    return readAt<unsigned int>(1*serial_size<unsigned int>());
}

unsigned int ClusterFile::get_last_empty() const
{
    return readAt<unsigned int>(3*serial_size<unsigned int>());
}

unsigned int ClusterFile::get_next(unsigned int where) const
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>() + serial_size<unsigned int>();
    return readAt<unsigned int>(pos);
}

unsigned int ClusterFile::get_previous(unsigned int where) const
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>();
    return readAt<unsigned int>(pos);
}

bool ClusterFile::get_occupied(unsigned int where) const
{
    std::streampos pos = where*linesize();
    return readAt<bool>(pos);
}

unsigned int  ClusterFile::get_refcount(unsigned int where) const
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>() + 2*serial_size<unsigned int>();
    return readAt<unsigned int>(pos);
}

void ClusterFile::set_first_filled(unsigned int what)
{
    writeAt(0,what);
}

void ClusterFile::set_first_empty(unsigned int what)
{
    writeAt(2*serial_size<unsigned int>(),what);
}

void ClusterFile::set_last_filled(unsigned int what)
{
    writeAt(1*serial_size<unsigned int>(),what);
}

void ClusterFile::set_last_empty(unsigned int what)
{
    writeAt(3*serial_size<unsigned int>(),what);
}

void ClusterFile::set_next(unsigned int where, unsigned int what)
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>() + serial_size<unsigned int>();
    writeAt(pos,what);
}

void ClusterFile::set_previous(unsigned int where, unsigned int what)
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>();
    writeAt(pos,what);
}

void ClusterFile::set_occupied(unsigned int where, bool what)
{
    std::streampos pos = where*linesize();
    writeAt(pos,what);
}

void ClusterFile::clear_refcount(unsigned int where)
{
    std::streampos pos = where*linesize();
    pos += serial_size<bool>()+2*serial_size<unsigned int>();
    writeAt<unsigned int>(pos,0);
}

void ClusterFile::set_refcount_add(unsigned int where, int add)
{
    unsigned int refcount = get_refcount(where);
    refcount+=add;
    std::streampos pos = where*linesize();
    pos += serial_size<bool>() + 2*serial_size<unsigned int>();
    writeAt(pos,refcount);
}

unsigned int ClusterFile::extend()
{
    if (!pre_open())
        return size();
    fs.seekp(0,fs.end);
    unsigned int new_space = fs.tellp() / linesize();
    fs << std::string(linesize()-2*serial_size<char>(), 0);
    fs << "\r\n";
    if (!fs.good()) {
        database->status = Database::Status::error;
        database->log << "Datei konnte nicht erweitert werden: " << name.fileOnly() << std::endl;
    }
    return new_space;
}

unsigned int ClusterFile::remove_empty()
{
    unsigned int first = get_first_empty();
    if (first == 0)
        return extend();
    unsigned int next = get_next(first);
    set_first_empty(next);
    if (next==0)
        set_last_empty(0);
    return first;
}

void ClusterFile::append_empty(unsigned int new_empty)
{
    unsigned int last = get_last_empty();
    if (last==0)
        set_first_empty(new_empty);
    else
        set_next(last,new_empty);
    set_last_empty(new_empty);
}

void ClusterFile::append_filled(unsigned int new_filled)
{
    unsigned int last = get_last_filled();
    if (last==0)
        set_first_filled(new_filled);
    else
        set_next(last,new_filled);
    set_last_filled(new_filled);
    clear_refcount(new_filled);
}

void ClusterFile::remove_filled(unsigned int to_remove)
{
    unsigned int next = get_next(to_remove);
    unsigned int previous = get_previous(to_remove);
    if (next==0)
        set_last_filled(previous);
    else
        set_previous(next,previous);
    if (previous==0)
        set_first_filled(next);
    else
        set_next(previous,next);
}

Cluster ClusterFile::emplace()
{
    unsigned int new_filled = remove_empty();
    append_filled(new_filled);
    set_occupied(new_filled,true);
    return create_cluster(new_filled);
}

void ClusterFile::erase(const Cluster& to_remove)
{
    unsigned int index = get_index(to_remove);
    if (get_refcount(index)>0)
        throw Exception("Removal of referenced Database-Element","Database::ClusterFile::erase");
    remove_filled(index);
    append_empty(index);
    set_occupied(index,false);
}

Cluster ClusterFile::at(unsigned int where)
{
    return create_cluster(where);
}

Cluster ClusterFile::next(const Cluster& current)
{
    return create_cluster(
        get_next(
            get_index(current)
        )
    );
}

Cluster ClusterFile::begin()
{
    return create_cluster(
        get_first_filled()
    );
}

const Cluster ClusterFile::at(unsigned int index) const
{
    return const_cast<ClusterFile*>(this)->at(index);
}

const Cluster ClusterFile::next(const Cluster& current) const
{
    return const_cast<ClusterFile*>(this)->next(current);
}

const Cluster ClusterFile::begin() const
{
    return const_cast<ClusterFile*>(this)->begin();
}

unsigned int ClusterFile::refcount(const Cluster& cluster)
{
    return get_refcount(
        get_index(cluster)
    );
}

void ClusterFile::add_refcount(const Cluster& cluster, int amount)
{
    set_refcount_add(
        get_index(cluster),
        amount
    );
}

bool ClusterFile::is_occupied(const Cluster& cluster)
{
    return get_occupied(
        get_index(cluster)
    );
}

std::streampos ClusterFile::linesize() const
{
    std::streampos min_out = 4*serial_size<unsigned int>() + 2;
    std::streampos out = type.size() + LINEHEAD;
    return std::max(out,min_out);
}

const std::streampos ClusterFile::LINEHEAD = 1*serial_size<bool>() + 3*serial_size<unsigned int>() + 2;