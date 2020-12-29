#ifndef TC_DATABASE_MEMBER_H
#define TC_DATABASE_MEMBER_H

#include "concepts.h"
#include "fileaccess.h"
#include "type.h"
#include <memory> // unique_ptr for operator ->

namespace tobilib {
namespace database_detail {

using MemberIterator = Iterator<Member>;
using const_MemberIterator = const_Iterator<Member>;

class Member : public Component, public Iteratable<Member>
{
public:
    Member() = default;
    Member(const MemberType&, const ListFile&, ListFile::LineIndex);
    Member(const MemberType&, const ClusterFile&, ClusterFile::LineIndex);

    bool operator==(const Member&) const;
    bool operator!=(const Member&) const;

    template<class PrimT>
    PrimT get() const;
    
    Cluster operator*() const;
    std::unique_ptr<Cluster> operator->() const;

    void set(int);
    void set(char);
    void set(const std::string&);
    void set(double);
    void set(bool);
    void set(const Cluster&);

    Member operator[](const std::string&);
    const Member operator[](const std::string&) const;
    Member operator[](unsigned int);
    const Member operator[](unsigned int) const;
    MemberIterator begin();
    MemberIterator end();
    void erase(const MemberIterator&);
    Member emplace();

    void clear_references();
    
TC_DATABASE_PRIVATE:
    void init_memory();
    ListFile::LineIndex get_list_begin() const;
    void set_list_begin(ListFile::LineIndex);

    MemberType type;
    File* fs;
    std::streampos position;
};

template<>
class Iterator<Member>
{
public:
    using ItemType = Member;

    MemberIterator& operator++();
    MemberIterator operator++(int);
    bool operator==(const MemberIterator&) const;
    bool operator!=(const MemberIterator&) const;
    Member operator*() const;
    Member* operator->() const;

TC_DATABASE_PRIVATE:
    void array_pp();
    void list_pp();

    mutable Member ref;
    Member parent;
};

} // namespace database_detail
} // namespace tobilib
#endif