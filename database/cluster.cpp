#define TC_DATABASE_INTERN
#include "database.h"

using namespace tobilib;
using namespace database_detail;

ClusterList::ClusterList(Database* db, ClusterFile* clfile):
    Component(db),
    cf(clfile)
{ }

Cluster ClusterList::operator[] (unsigned int index)
{
    if (!pre_open("ClusterList::operator[]"))
        return Cluster();
    return Cluster(database,cf,index);
}

const Cluster ClusterList::operator[] (unsigned int index) const
{
    return const_cast<ClusterList*>(this)->operator[](index);
}

Cluster ClusterList::emplace()
{
    if (!pre_open("ClusterList::emplace()"))
        return Cluster();
    Cluster out (database,cf,cf->emplace());
    out.init_memory();
    return out;
}

void ClusterList::erase(const ClusterIterator& where)
{
    if (!pre_open("ClusterList::erase()"))
        return;
    where->erase();
}

ClusterIterator ClusterList::begin()
{
    if (!pre_open("ClusterList::begin()"))
        return end();
    ClusterIterator out;
    out.ref = Cluster(database,cf,cf->get_first_filled());
    return out;
}

ClusterIterator ClusterList::end()
{
    return ClusterIterator();
}

Cluster::Cluster(Database* db, ClusterFile* clfile, LineIndex ln):
    Component(db),
    cf(clfile),
    line(ln)
{
    if (!pre_open("Cluster::Constructor") || ln==0 || ln>=cf->get_data_capacity() || !cf->get_occupied(ln))
    {
        line = 0;
        nullflag = true;
    }
}

bool Cluster::operator==(const Cluster& other) const
{
    if (nullflag && other.nullflag)
        return true;
    return cf==other.cf && line==other.line;
}

bool Cluster::operator!=(const Cluster& other) const
{
    return !(*this==other);
}

Member Cluster::operator[] (const std::string& name)
{
    if (nullflag)
        return Member();
    return Member(cf->type.getMember(name),*cf,index());
}

Member Cluster::operator[] (const MemberType& memtype)
{
    if (nullflag)
        return Member();
    return Member(memtype,*cf,index());
}

unsigned int Cluster::index() const
{
    if (!pre_valid("Cluster::index()"))
        return 0;
    return line;
}

void Cluster::erase()
{
    if (!pre_valid("Cluster::erase()"))
        return;
    clear_references();
    cf->erase(line);
    nullflag=true;
}

unsigned int Cluster::reference_count()
{
    if (!pre_valid("Cluster::reference_count()"))
        return 0;
    return cf->get_refcount(line);
}

void Cluster::clear_references()
{
    if (!pre_valid("Cluster::clear_references()"))
        return;
    for (MemberType& item: cf->type.members)
        (*this)[item.name].clear_references();
}

bool Cluster::pre_valid(const std::string& trace) const
{
    if (!pre_open(trace))
        return false;
    if (!cf->get_occupied(line))
        throw Exception("Zugriff auf entferntes Element","Cluster::pre_valid()");
    return true;
}

void Cluster::init_memory()
{
    for (MemberType& m: cf->type.members)
        (*this)[m.name].init_memory();
}

void Cluster::add_refcount(int amount)
{
    if (!pre_good())
        return;
    cf->set_refcount_add(line,amount);
}

ClusterIterator& ClusterIterator::operator++()
{
    if (!ref.pre_valid("ClusterIterator::operator++"))
        return *this;
    ref = Cluster(
        ref.database,
        ref.cf,
        ref.cf->get_next(ref.line)
    );
    return *this;
}

ClusterIterator ClusterIterator::operator++(int)
{
    ClusterIterator out = *this;
    ++*this;
    return out;
}

bool ClusterIterator::operator==(const ClusterIterator& other) const
{
    return ref==other.ref;
}

bool ClusterIterator::operator!=(const ClusterIterator& other) const
{
    return !(*this==other);
}

Cluster ClusterIterator::operator*() const
{
    return ref;
}

Cluster* ClusterIterator::operator->() const
{
    return &ref;
}