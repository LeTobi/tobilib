#ifndef TC_DATABASE_CONCEPTS_H
#define TC_DATABASE_CONCEPTS_H

#include "../stringplus/filename.h"

namespace tobilib{

class Database;

namespace database_detail{

class BlockType;
class MemberType;
class ClusterType;
class Member;
class Cluster;
class Parser;


class Component {
public:
    Component (Database*);
    Component ();

    mutable bool nullflag;
    bool pre_good() const;
    bool pre_init() const;
    bool pre_open() const;

protected:
    Database* database;
};

template <class ComponentType>
class Iterator { };

template <class ComponentType>
class const_Iterator
{
private:
    using BaseIterator = Iterator<ComponentType>;
    BaseIterator it;
public:
    const_Iterator() {};
    const_Iterator(const BaseIterator& other): it(other) {};
    const_Iterator& operator++() {it++; return *this;};
    const_Iterator operator++(int){return it++;};
    bool operator==(const const_Iterator& other) const {return it==other.it;};
    bool operator!=(const const_Iterator& other) const {return it!=other.it;};
    const typename BaseIterator::ItemType operator*() const {return *it;};
    const typename BaseIterator::ItemType operator->() const {return *it;};
};

template <class ComponentType>
class Iteratable
{
    using IteratorType = Iterator<ComponentType>;
    using const_IteratorType = const_Iterator<ComponentType>;
    virtual IteratorType begin() = 0;
    virtual IteratorType end() = 0;
    const_IteratorType begin() const;
    const_IteratorType end() const;
    const_IteratorType cbegin() const;
    const_IteratorType cend() const;
};

} // namespace database_detail
} // namespace tobilib

#endif