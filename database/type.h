#ifndef TC_DATABASE_TYPE_H
#define TC_DATABASE_TYPE_H

#include <ios>
#include <map>

namespace tobilib {
namespace database_detail {

class BlockType
{
public:
    std::streampos size;

    BlockType(): id(0), size(0) {};

    bool operator==(const BlockType& other) const {return other.id==id;};
    bool operator!=(const BlockType& other) const {return !(*this==other);};

    const static BlockType t_int;
    const static BlockType t_char;
    const static BlockType t_double;
    const static BlockType t_bool;
    const static BlockType t_list;
    const static BlockType t_ptr;

private:
    unsigned int id;
    BlockType(unsigned int _id, std::streampos _size): id(_id), size(_size) {};
};

class MemberType {
public:
    bool operator==(const MemberType&) const;
    bool operator!=(const MemberType&) const;

    BlockType blockType;
    unsigned int amount;
    ClusterType* ptr_type = nullptr;

    std::streampos size() const;
};

class ClusterType
{
public:
    std::string name;
    std::map<std::string,MemberType> members;

    bool operator==(const ClusterType&) const;
    bool operator!=(const ClusterType&) const;
    std::streampos size() const;
    std::streampos offsetOf(const std::string&) const;
};

} // namespace database_detail
} // namespace database_datail

#endif