#include "database.h"

using namespace tobilib;
using namespace database_detail;

const BlockType BlockType::t_int   (1, serial_size<int>());
const BlockType BlockType::t_char  (2, serial_size<char>());
const BlockType BlockType::t_double(3, serial_size<double>());
const BlockType BlockType::t_bool  (4, serial_size<bool>());
const BlockType BlockType::t_list  (5, serial_size<unsigned int>());
const BlockType BlockType::t_ptr   (6, serial_size<unsigned int>());

bool MemberType::operator==(const MemberType& other) const
{
    if (this==&other)
        return true;
    return
        name == other.name &&
        parent_offset == other.parent_offset &&
        blockType == other.blockType &&
        amount == other.amount &&
        ptr_type == other.ptr_type;
}

bool MemberType::operator!=(const MemberType& other) const
{
    return !(*this==other);
}

bool Database::ClusterType::operator== (const ClusterType& other) const
{
    if (this == &other)
        return true;
    if (members.size() != other.members.size())
        return false;
    for (int i=0;i<members.size();i++)
        if (members[i] != other.members[i])
            return false;
    return true;
}

bool Database::ClusterType::operator!= (const ClusterType& other) const
{
    return !(*this == other);
}

bool Database::ClusterType::contains(const std::string& name) const
{
    for (const MemberType& mem: members) {
        if (mem.name==name)
            return true;
    }
    return false;
}

MemberType& Database::ClusterType::getMember(const std::string& name)
{
    for (MemberType& mem: members) {
        if (mem.name==name)
            return mem;
    }
    throw Exception("Name nicht gefunden","Database::ClusterType::operator[]");
}

const MemberType& Database::ClusterType::getMember(const std::string& name) const
{
    return const_cast<ClusterType*>(this)->getMember(name);
}