#ifndef TC_DATABASE_CONCEPTS_H
#define TC_DATABASE_CONCEPTS_H

#ifdef TC_DATABASE_INTERN
    #define TC_DATABASE_PRIVATE public
#else
    #define TC_DATABASE_PRIVATE private
#endif

#include "../stringplus/filename.h"

namespace tobilib{

class Database;

namespace database_detail{

using LineIndex = unsigned int;
using filesize_t = unsigned long int; // should be = off_t

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

    bool is_null() const;
    bool pre_good() const;
    bool pre_init(const std::string&) const;
    bool pre_open(const std::string&) const;

TC_DATABASE_PRIVATE:
    bool nullflag;
    Database* database;
};

template <class ComponentType>
class Iterator { };

template <class ComponentType>
class const_Iterator
{
TC_DATABASE_PRIVATE:
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
public:
    using IteratorType = Iterator<ComponentType>;
    using const_IteratorType = const_Iterator<ComponentType>;
    virtual IteratorType begin() = 0;
    virtual IteratorType end() = 0;
    const_IteratorType begin() const;
    const_IteratorType end() const;
    const_IteratorType cbegin() const;
    const_IteratorType cend() const;
};

template<class ComponentType>
const_Iterator<ComponentType> Iteratable<ComponentType>::begin() const
{
    return const_cast<Iteratable<ComponentType>*>(this)->begin();
}

template<class ComponentType>
const_Iterator<ComponentType> Iteratable<ComponentType>::end() const
{
    return const_cast<Iteratable<ComponentType>*>(this)->end();
}

template<class ComponentType>
const_Iterator<ComponentType> Iteratable<ComponentType>::cbegin() const
{
    return const_cast<Iteratable<ComponentType>*>(this)->begin();
}

template<class ComponentType>
const_Iterator<ComponentType> Iteratable<ComponentType>::cend() const
{
    return const_cast<Iteratable<ComponentType>*>(this)->end();
}

} // namespace database_detail
} // namespace tobilib

#endif