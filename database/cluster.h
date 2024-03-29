#ifndef TC_DATABASE_CLUSTER_H
#define TC_DATABASE_CLUSTER_H

#include "concepts.h"
#include "type.h"
#include "filetable.h"

namespace tobilib {
namespace database_detail {

using ClusterIterator = Iterator<Cluster>;
using const_ClusterIterator = const_Iterator<ClusterIterator>;

class ClusterList : public Component, public Iteratable<Cluster>
{
public:
    ClusterList() = default;
    ClusterList(Database*, ClusterFile*);

    Cluster operator[](unsigned int);
    const Cluster operator[](unsigned int) const;
    Cluster emplace();
    void erase(const ClusterIterator&);
    ClusterIterator begin();
    ClusterIterator end();
    const ClusterType& type() const;

TC_DATABASE_PRIVATE:
    ClusterFile* cf;
};

class Cluster : public Component
{
public:
    Cluster() = default;
    Cluster(Database*, ClusterFile*, LineIndex);

    // diese referenz wird ungültig mit database.init()
    const ClusterType& type() const;

    bool operator==(const Cluster& other) const;
    bool operator!=(const Cluster& other) const;
    bool operator<(const Cluster& other) const; // nützlich in sets und maps

    Member operator[](const std::string&);
    Member operator[](const MemberType&);
    const Member operator[](const std::string&) const;
    const Member operator[](const MemberType&) const;
    unsigned int index() const;
    void erase();
    unsigned int reference_count();
    void clear_references();

TC_DATABASE_PRIVATE:
    friend ClusterList;
    friend ClusterIterator;
    friend Member;

    bool pre_valid(const std::string&) const;
    void init_memory();
    void add_refcount(int);

    ClusterFile* cf;
    LineIndex line;
};

template<>
class Iterator<Cluster>
{
public:
    using ItemType = Cluster;

    ClusterIterator& operator++();
    ClusterIterator operator++(int);
    bool operator==(const ClusterIterator&) const;
    bool operator!=(const ClusterIterator&) const;
    Cluster operator*() const;
    Cluster* operator->() const;

TC_DATABASE_PRIVATE:
    friend ClusterList;

    mutable Cluster ref;
};

} // namespace database_detail
} // namespace tobilib
#endif