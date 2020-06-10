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
    return cf->at(index);
}

const Cluster ClusterList::operator[] (unsigned int index) const
{
    return const_cast<ClusterList*>(this)->operator[](index);
}

Cluster ClusterList::emplace()
{
    if (!pre_open())
        return Cluster();
    Cluster out = cf->emplace();
    out.init();
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
    out.ref = cf->begin();
    return out;
}

ClusterIterator ClusterList::end()
{
    return ClusterIterator();
}

Cluster::Cluster(Database* db, ClusterFile* clfile, std::streampos pos):
    Component(db),
    cf(clfile),
    position(pos)
{ }

bool Cluster::operator==(const Cluster& other) const
{
    if (nullflag && other.nullflag)
        return true;
    return cf==other.cf && position==other.position;
}

bool Cluster::operator!=(const Cluster& other) const
{
    return !(*this==other);
}

Member Cluster::operator() (const std::string& name)
{
    if (!pre_valid())
        return Member();
    if (cf->type.members.count(name)==0)
        throw Exception("Member not found","Database::Cluster:operator()");
    return Member(
        database,
        cf->type.members.at(name),
        cf,
        position+cf->type.offsetOf(name)
    );
}

unsigned int Cluster::index() const
{
    if (!pre_valid())
        return 0;
    return cf->get_index(*this);
}

void Cluster::erase()
{
    if (!pre_valid())
        return;
    clear_references();
    cf->erase(*this);
    nullflag=true;
}

unsigned int Cluster::reference_count()
{
    if (!pre_valid())
        return 0;
    return cf->refcount(*this);
}

void Cluster::clear_references()
{
    if (!pre_valid())
        return;
    for (auto& item: cf->type.members)
        (*this)(item.first).clear_references();
}

bool Cluster::pre_valid() const
{
    if (!pre_open())
        return false;
    if (!cf->is_occupied(*this)) {
        database->status = Database::Status::error;
        database->log << "Zugriff auf entferntes element" << std::endl;
        return false;
    }
    return true;
}

void Cluster::add_refcount(int amount)
{
    if (nullflag)
        return;
    cf->add_refcount(*this,amount);
}

void Cluster::init()
{
    for (auto& m: cf->type.members)
        (*this)(m.first).init();
}

ClusterIterator& ClusterIterator::operator++()
{
    ref = ref.cf->next(ref);
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