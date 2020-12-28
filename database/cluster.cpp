#include "database.h"

using namespace tobilib;
using namespace database_detail;

ClusterList::ClusterList(Database* db, ClusterFile* clfile):
    Component(db),
    cf(clfile)
{ }

Cluster ClusterList::operator[] (unsigned int index)
{
    if (!pre_open())
        return Cluster();
    return Cluster(database,cf,index);
}

const Cluster ClusterList::operator[] (unsigned int index) const
{
    return const_cast<ClusterList*>(this)->operator[](index);
}

Cluster ClusterList::emplace()
{
    if (!pre_open())
        return Cluster();
    Cluster out (database,cf,cf->emplace());
    out.init_memory();
    return out;
}

void ClusterList::erase(const ClusterIterator& where)
{
    if (!pre_open())
        return;
    where->erase();
}

ClusterIterator ClusterList::begin()
{
    if (!pre_open())
        return end();
    ClusterIterator out;
    out.ref = Cluster(database,cf,cf->get_first_filled());
    return out;
}

ClusterIterator ClusterList::end()
{
    return ClusterIterator();
}

Cluster::Cluster(Database* db, ClusterFile* clfile, ClusterFile::LineIndex ln):
    Component(db),
    cf(clfile),
    line(ln)
{
    if (!pre_open() || ln==0 || ln>cf->capacity() || !cf->get_occupied(ln))
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
    if (!pre_valid())
        return Member();
    if (cf->type.members.count(name)==0)
        throw Exception("Member not found","Database::Cluster:operator()");
    return Member(
        database,
        cf,
        cf->type.members.at(name),
        cf->data_location(line) + cf->type.offsetOf(name)
    );
}

unsigned int Cluster::index() const
{
    if (!pre_valid())
        return 0;
    return line;
}

void Cluster::erase()
{
    if (!pre_valid())
        return;
    clear_references();
    cf->erase(line);
    nullflag=true;
}

unsigned int Cluster::reference_count()
{
    if (!pre_valid())
        return 0;
    return cf->get_refcount(line);
}

void Cluster::clear_references()
{
    if (!pre_valid())
        return;
    for (auto& item: cf->type.members)
        (*this)[item.first].clear_references();
}

bool Cluster::pre_valid() const
{
    if (!pre_open())
        return false;
    if (!cf->get_occupied(line))
        throw Exception("Zugriff auf entferntes Element","Cluster::pre_valid()");
    return true;
}

void Cluster::init_memory()
{
    for (auto& m: cf->type.members)
        (*this)[m.first].init_memory();
}

void Cluster::add_refcount(int amount)
{
    if (nullflag)
        return;
    cf->set_refcount_add(line,amount);
}

ClusterIterator& ClusterIterator::operator++()
{
    if (!ref.pre_valid())
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