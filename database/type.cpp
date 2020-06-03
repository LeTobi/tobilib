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
    return
        blockType==other.blockType &&
        amount == other.amount &&
        ptr_type==other.ptr_type;
}

bool MemberType::operator!=(const MemberType& other) const
{
    return !(*this==other);
}

std::streampos MemberType::size() const
{
    return amount*blockType.size;
}

bool Database::ClusterType::operator== (const ClusterType& other) const
{
    return name==other.name;
}

bool Database::ClusterType::operator!= (const ClusterType& other) const
{
    return !(*this == other);
}

std::streampos Database::ClusterType::size() const
{
    std::streampos out = 0;
    for (auto& mem: members) {
        out+=mem.second.size();
    }
    return out;
}

std::streampos Database::ClusterType::offsetOf(const std::string& name) const
{
    if (members.count(name)==0)
        throw Exception("Implementierungsfehler","Database::ClusterType::offsetOf");
    std::streampos out = 0;
    for (auto& mem: members) {
        if (mem.first==name)
            return out;
        out+=mem.second.size();
    }
    // never reached!
    return 0;
}