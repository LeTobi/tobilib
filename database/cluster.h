#ifndef TC_DATABASE_CLUSTER_H
#define TC_DATABASE_CLUSTER_H

#include "concepts.h"
#include "type.h"
#include "fileaccess.h"

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

private:
    ClusterFile* cf;
};

class Cluster : public Component
{
public:
    Cluster() = default;
    Cluster(Database*, ClusterFile*, std::streampos);

    bool operator==(const Cluster& other) const;
    bool operator!=(const Cluster& other) const;

    Member operator()(const std::string&);
    const Member operator()(const std::string&) const;
    unsigned int index() const;
    void erase();
    unsigned int reference_count();
    void clear_references();
    
    bool pre_valid() const;

private:
    friend class ClusterFile;
    friend class ClusterList;
    friend class Iterator<Cluster>;
    friend class Member;

    void add_refcount(int);
    void init();

    ClusterFile* cf;
    std::streampos position;
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

private:
    friend class ClusterList;

    mutable Cluster ref;
};

} // namespace database_detail
} // namespace tobilib
#endif