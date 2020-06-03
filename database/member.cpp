#include "database.h"

using namespace tobilib;
using namespace database_detail;

Member::Member(
    Database* db,
    const MemberType& t,
    File* f,
    std::streampos pos
    ):
        Component(db),
        type(t),
        fs(f),
        position(pos)
{ }

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

Member::operator int() const
{
    if (!pre_open())
        return 0;
    if (type.blockType!=BlockType::t_int)
        throw ("Type-Error","Database::Member::operator int");
    return fs->readAt<int>(position);
}

Member::operator char() const
{
    if (!pre_open())
        return 0;
    if (type.blockType!=BlockType::t_char)
        throw ("Type-Error","Database::Member::operator char");
    return fs->readAt<char>(position);
}

Member::operator std::string() const
{
    if (!pre_open())
        return "";
    if (type.blockType!=BlockType::t_char)
        throw ("Type-Error","Database::Member::operator std::string");
    std::string out;
    for (std::streampos i=0;i<type.amount;i+=1)
    {
        char c = fs->readAt<char>(position+i);
        if (c==0)
            break;
        out+=c;
    }
    return out;
}

Member::operator double() const
{
    if (!pre_open())
        return 0;
    if (type.blockType!=BlockType::t_double)
        throw ("Type-Error","Database::Member::operator double");
    return fs->readAt<double>(position);
}

Member::operator bool() const
{
    if (!pre_open())
        return 0;
    if (type.blockType!=BlockType::t_bool)
        throw ("Type-Error","Database::Member::operator bool");
    return fs->readAt<bool>(position);
}

Cluster Member::operator*() const
{
    if (!pre_open())
        return Cluster();
    if (type.blockType != BlockType::t_ptr)
        throw Exception("Type-Error","Database::Member::operator*");
    unsigned int index = fs->readAt<unsigned int>(position);
    return database->get_cluster(type.ptr_type)->at(index);
}

std::unique_ptr<Cluster> Member::operator->() const
{
    std::unique_ptr<Cluster> out (new Cluster(**this));
    return out;
}

void Member::operator= (int out)
{
    if (!pre_open())
        return;
    if (type.blockType!=BlockType::t_int)
        throw Exception("Type-Error","Database::Member::operator= int");
    fs->writeAt(position,out);
}

void Member::operator= (char out)
{
    if (!pre_open())
        return;
    if (type.blockType!=BlockType::t_char)
        throw Exception("Type-Error","Database::Member::operator= char");
    fs->writeAt(position,out);
}

void Member::operator= (const std::string& out)
{
    if (!pre_open())
        return;
    if (type.blockType!=BlockType::t_char)
        throw Exception("Type-Error","Database::Member::operator= std::string");
    for (std::streampos i=0;i<type.amount;i+=1)
    {
        if (i<out.size())
            fs->writeAt(position+i,out[i]);
        else
            fs->writeAt<char>(position+i,0);
    }
}

void Member::operator= (double out)
{
    if (!pre_open())
        return;
    if (type.blockType!=BlockType::t_double)
        throw Exception("Type-Error","Database::Member::operator= double");
    fs->writeAt(position,out);
}

void Member::operator= (bool out)
{
    if (!pre_open())
        return;
    if (type.blockType!=BlockType::t_bool)
        throw Exception("Type-Error","Database::Member::operator= bool");
    fs->writeAt(position,out);
}

void Member::operator=(const Cluster& cluster)
{
    if (!pre_open())
        return;
    if (type.blockType!=BlockType::t_ptr)
        throw Exception("Type-Error","Database::Member::operator= Element");
    if (!cluster.nullflag && cluster.cf->type != *type.ptr_type)
        throw Exception("Type-Error (Cluster Mismatch)","Database::Member::operator= Cluster");
    (*this)->add_refcount(-1);
    fs->writeAt(position,cluster.index());
    (*this)->add_refcount(1);
}

Member Member::operator()(const std::string& name)
{
    return (*this)->operator()(name);
}

const Member Member::operator()(const std::string& name) const
{
    return const_cast<Member*>(this)->operator()(name);
}

Member Member::operator[](unsigned int index)
{
    if (!pre_open())
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
    if (!pre_open())
        return end();
    MemberIterator out;
    out.parent = *this;
    if (type.blockType==BlockType::t_list)
        out.ref = database->listfile.begin(*this);
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
    if (!pre_open())
        return;
    if (where.parent != *this)
        throw Exception("Das Element gehoert nicht zur liste","Database::Member::erase");
    database->listfile.erase(*this,*where);
}

Member Member::emplace()
{
    if (!pre_open())
        return Member();
    if (type.blockType != BlockType::t_list)
        throw Exception("Type-Error","Database::Member::emplace()");
    Member out = database->listfile.emplace(*this);
    out.init();
    return out;
}

void Member::clear_references()
{
    if (!pre_open())
        return;
    if (type.amount>1) {
        for (auto m: (*this))
            m.clear_references();
    }
    else if (type.blockType==BlockType::t_ptr) {
        *this = Cluster();
    }
    else if (type.blockType==BlockType::t_list) {
        while (begin()!=end()) {
            begin()->clear_references();
            erase(begin());
        }
    }
}

void Member::init()
{
    if (!pre_open())
        return;
    if (type.amount>1) {
        for (auto m: (*this))
            m.init();
    }
    else if (type.blockType==BlockType::t_ptr) {
        fs->writeAt<unsigned int>(position,0);
    }
    else if (type.blockType==BlockType::t_list) {
        fs->writeAt<unsigned int>(position,0);
    }
    // hier könnte noch mehr initialisiert werden
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
    std::streampos end = parent.position+parent.type.size();
    std::streampos increment = ref.type.blockType.size;
    ref.position += increment;
    if (ref.position>=end)
        ref.nullflag = true;
}

void MemberIterator::list_pp()
{
    ref = ref.database->listfile.next(parent,ref);
}