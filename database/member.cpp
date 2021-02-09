#define TC_DATABASE_INTERN
#include "database.h"

using namespace tobilib;
using namespace database_detail;

Member::Member(const MemberType& listtype, const ListFile& listfile, LineIndex idx):
    Component(listfile.database)
{
    if (idx==0)
    {
        nullflag = true;
        position = 0;
        return;
    }

    if (listtype.blockType != BlockType::t_list)
        throw Exception("Keine Liste vorhanden","Database::Member constructor");

    fs = &database->listfile;

    type.name = "";
    type.parent = nullptr;
    type.parent_offset = 0l;
    type.blockType = BlockType::t_ptr;
    type.amount = 1;
    type.ptr_type = listtype.ptr_type;
    type.size = BlockType::t_ptr.size;
    
    position = idx * ListFile::LINESIZE + ListFile::LINEHEAD;
}

Member::Member(const MemberType& t, const ClusterFile& clusterfile, LineIndex idx):
        Component(clusterfile.database)
{
    if (idx==0)
    {
        nullflag = true;
        position = 0;
        return;
    }

    if (clusterfile.type != *t.parent)
        throw Exception("Typ enthaelt die Eigenschaft nicht","Database::Member constructor");

    fs = (CrashSafeFile*)&clusterfile;
    type = t;
    position = clusterfile.data_location(idx) + t.parent_offset;
}

bool Member::operator==(const Member& other) const
{
    if (nullflag && other.nullflag)
        return true;
    return
        fs == other.fs &&
        position == other.position;
}

bool Member::operator!=(const Member& other) const
{
    return !(*this==other);
}

template<>
int Member::get() const
{
    if (!pre_open("Member::get<int>()"))
        return 0;
    if (type.blockType!=BlockType::t_int)
        throw ("Type-Error","Database::Member::get<int>");
    return fs->readAt<int>(position);
}

template<>
char Member::get() const
{
    if (!pre_open("Member::get<char>()"))
        return 0;
    if (type.blockType!=BlockType::t_char)
        throw ("Type-Error","Database::Member::get<char>");
    return fs->readAt<char>(position);
}

template<>
std::string Member::get() const
{
    if (!pre_open("Member::get<string>()"))
        return "";
    if (type.blockType!=BlockType::t_char)
        throw ("Type-Error","Database::Member::get<string>");
    std::string out;
    for (filesize_t i=0;i<type.amount;i+=1)
    {
        char c = fs->readAt<char>(position+i);
        if (c==0)
            break;
        out+=c;
    }
    return out;
}

template<>
double Member::get() const
{
    if (!pre_open("Member::get<double>()"))
        return 0;
    if (type.blockType!=BlockType::t_double)
        throw ("Type-Error","Database::Member::get<double>");
    return fs->readAt<double>(position);
}

template<>
bool Member::get() const
{
    if (!pre_open("Member::get<bool>()"))
        return 0;
    if (type.blockType!=BlockType::t_bool)
        throw ("Type-Error","Database::Member::get<bool>");
    return fs->readAt<bool>(position);
}

Cluster Member::operator*() const
{
    if (!pre_open("Member::get<Cluster>()"))
        return Cluster();
    if (type.blockType != BlockType::t_ptr)
        throw Exception("Type-Error","Database::Member::operator*");
    unsigned int index = fs->readAt<unsigned int>(position);
    return Cluster(
        database,
        database->get_file(type.ptr_type),
        index
    );
}

std::unique_ptr<Cluster> Member::operator->() const
{
    std::unique_ptr<Cluster> out (new Cluster(**this));
    return out;
}

void Member::set(int out)
{
    if (!pre_open("Member::set(int)"))
        return;
    if (type.blockType!=BlockType::t_int)
        throw Exception("Type-Error","Database::Member::set(int)");
    fs->writeAt(position,out);
}

void Member::set(char out)
{
    if (!pre_open("Member::set(char)"))
        return;
    if (type.blockType!=BlockType::t_char)
        throw Exception("Type-Error","Database::Member::set(char)");
    fs->writeAt(position,out);
}

void Member::set(const std::string& out)
{
    if (!pre_open("Member::set(string)"))
        return;
    if (type.blockType!=BlockType::t_char)
        throw Exception("Type-Error","Database::Member::set(string)");
    for (filesize_t i=0;i<type.amount;i+=1)
    {
        if (i<out.size())
            fs->writeAt(position+i,out[i]);
        else
            fs->writeAt<char>(position+i,0);
    }
}

void Member::set(double out)
{
    if (!pre_open("Member::set(double)"))
        return;
    if (type.blockType!=BlockType::t_double)
        throw Exception("Type-Error","Database::Member::set(double)");
    fs->writeAt(position,out);
}

void Member::set(bool out)
{
    if (!pre_open("Member::set(bool)"))
        return;
    if (type.blockType!=BlockType::t_bool)
        throw Exception("Type-Error","Database::Member::set(bool)");
    fs->writeAt(position,out);
}

void Member::set(const Cluster& cluster)
{
    if (!pre_open("Member:set(Cluster)"))
        return;
    if (type.blockType!=BlockType::t_ptr)
        throw Exception("Type-Error","Database::Member::set(Cluster)");
    if (!cluster.nullflag && cluster.cf->type != *type.ptr_type)
        throw Exception("Type-Error (Cluster Mismatch)","Database::Member::set(cluster)");
    FlagRequest lock = database->begin_critical_operation();
        (*this)->add_refcount(-1);
        fs->writeAt(position,cluster.index());
        (*this)->add_refcount(1);
    database->end_critical_operation(lock);
}

Member Member::operator[](const std::string& name)
{
    return (*this)->operator[](name);
}

const Member Member::operator[](const std::string& name) const
{
    return const_cast<Member*>(this)->operator[](name);
}

Member Member::operator[](unsigned int index)
{
    if (!pre_open("Member::operator[int]"))
        return Member();
    if (index>=type.amount)
        throw Exception("out of range","Database::Member::operator[]");
    Member out(*this);
    out.type.amount=1;
    out.position+=index*type.blockType.size;
    return out;
}

const Member Member::operator[](unsigned int index) const
{
    return const_cast<Member*>(this)->operator[](index);
}

MemberIterator Member::begin()
{
    if (!pre_open("MemberIterator::begin()"))
        return end();
    MemberIterator out;
    out.parent = *this;
    if (type.blockType==BlockType::t_list)
        out.ref = Member(type,database->listfile,get_list_begin());
    else
        out.ref = (*this)[0];
    return out;
}

MemberIterator Member::end()
{
    return MemberIterator();
}

void Member::erase(const MemberIterator& where)
{
    if (!pre_open("Member::erase()"))
        return;
    if (is_null() || where.ref.is_null())
        return;
    if (type.blockType != BlockType::t_list)
        throw Exception("Type error, Liste erwartet.","Database::Member::erase");
    if (where.parent != *this)
        throw Exception("Das Element gehoert nicht zur liste","Database::Member::erase");

    FlagRequest lock = database->begin_critical_operation();
        where->clear_references();
        LineIndex target = get_list_begin();
        LineIndex to_remove = database->listfile.get_index(where.ref.position);
        LineIndex successor = database->listfile.erase(to_remove);
        if (target==to_remove)
            set_list_begin(successor);
    database->end_critical_operation(lock);
}

Member Member::emplace()
{
    if (!pre_open("Member::emplace()"))
        return Member();
    if (type.blockType != BlockType::t_list)
        throw Exception("Type-Error","Database::Member::emplace()");

    FlagRequest lock = database->begin_critical_operation();
        set_list_begin(
            database->listfile.emplace(get_list_begin())
        );
        Member out = Member(type,database->listfile,get_list_begin());
        out.init_memory();
    database->end_critical_operation(lock);
    
    return out;
}

void Member::clear_references()
{
    if (!pre_open("Member::clear_references()"))
        return;
    if (type.amount>1) {
        for (auto m: (*this))
            m.clear_references();
    }
    else if (type.blockType==BlockType::t_ptr) {
        this->set(Cluster());
    }
    else if (type.blockType==BlockType::t_list) {
        while (begin()!=end())
            erase(begin());
    }
}

void Member::init_memory()
{
    if (!pre_open("Member::init_memory()"))
        return;
    if (type.amount>1) {
        for (auto m: (*this))
            m.init_memory();
    }
    else if (type.blockType==BlockType::t_ptr) {
        fs->writeAt<unsigned int>(position,0);
    }
    else if (type.blockType==BlockType::t_list) {
        fs->writeAt<unsigned int>(position,0);
    }
}

LineIndex Member::get_list_begin() const
{
    if (!pre_open("Member::get_list_begin"))
        return 0;
    if (type.blockType != BlockType::t_list)
        throw Exception("Das Element ist keine Liste.","Member::get_list_begin()");
    return fs->readAt<unsigned int>(position);
}

void Member::set_list_begin(LineIndex index)
{
    if (!pre_open("Member::set_list_begin()"))
        return;
    if (type.blockType != BlockType::t_list)
        throw Exception("Das Element ist keine Liste.","Member::set_list_begin()");
    fs->writeAt(position,index);
}

MemberIterator& MemberIterator::operator++()
{
    if (parent.nullflag)
        return *this;
    if (parent.type.blockType == BlockType::t_list)
        list_pp();
    else
        array_pp();
    return *this;
}

MemberIterator MemberIterator::operator++(int)
{
    MemberIterator out = *this;
    ++*this;
    return out;
}

bool MemberIterator::operator==(const MemberIterator& other) const
{
    return ref == other.ref;
}

bool MemberIterator::operator!=(const MemberIterator& other) const
{
    return !(*this==other);
}

Member MemberIterator::operator*() const
{
    return ref;
}

Member* MemberIterator::operator->() const
{
    return &ref;
}

void MemberIterator::array_pp()
{
    filesize_t end = parent.position+parent.type.size;
    filesize_t increment = ref.type.blockType.size;
    ref.position += increment;
    if (ref.position>=end)
    {
        ref.nullflag = true;
        ref.position = 0;
    }
}

void MemberIterator::list_pp()
{
    if (!ref.pre_open("MemberIterator::list_pp()"))
    {
        ref = Member();
        return;
    }
    LineIndex index = ref.database->listfile.get_index(ref.position);
    index = ref.database->listfile.get_next(index);
    ref = Member(parent.type,parent.database->listfile,index);
}