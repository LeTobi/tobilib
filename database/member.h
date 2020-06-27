#ifndef TC_DATABASE_MEMBER_H
#define TC_DATABASE_MEMBER_H

#include "concepts.h"
#include "fileaccess.h"
#include <memory> // unique_ptr for operator ->

namespace tobilib {
namespace database_detail {

using MemberIterator = Iterator<Member>;
using const_MemberIterator = const_Iterator<Member>;

class Member : public Component, public Iteratable<Member>
{
public:
    Member() = default;
    Member(Database*,const MemberType&,File*,std::streampos);

    bool operator==(const Member&) const;
    bool operator!=(const Member&) const;

    operator int() const;
    operator char() const;
    operator std::string() const;
    operator double() const;
    operator bool() const;
    Cluster operator*() const;
    std::unique_ptr<Cluster> operator->() const;

    void operator=(int);
    void operator=(char);
    void operator=(const std::string&);
    void operator=(double);
    void operator=(bool);
    void operator=(const Cluster&);

    Member operator()(const std::string&);
    const Member operator()(const std::string&) const;
    Member operator[](unsigned int);
    const Member operator[](unsigned int) const;
    MemberIterator begin();
    MemberIterator end();
    void erase(const MemberIterator&);
    Member emplace();

    void clear_references();
    
private:
    friend class ListFile;
    friend class Cluster;
    friend class Iterator<Member>;

    void init();

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

private:
    friend class Member;

    void array_pp();
    void list_pp();

    mutable Member ref;
    Member parent;
};

} // namespace database_detail
} // namespace tobilib
#endif