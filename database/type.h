#ifndef TC_DATABASE_TYPE_H
#define TC_DATABASE_TYPE_H

#include <ios>
#include <vector>
#include "concepts.h"

namespace tobilib {
namespace database_detail {

class BlockType
{
public:
    filesize_t size;

    BlockType(): id(0), size(0) {};

    bool operator==(const BlockType& other) const {return other.id==id;};
    bool operator!=(const BlockType& other) const {return !(*this==other);};

    const static BlockType t_invalid;
    const static BlockType t_int;
    const static BlockType t_char;
    const static BlockType t_double;
    const static BlockType t_bool;
    const static BlockType t_list;
    const static BlockType t_ptr;

private:
    unsigned int id;
    BlockType(unsigned int _id, filesize_t _size): id(_id), size(_size) {};
};

class MemberType {
public:
    bool operator==(const MemberType&) const;
    bool operator!=(const MemberType&) const;

    std::string name;
    ClusterType* parent;
    filesize_t parent_offset = 0;
    BlockType blockType;
    unsigned int amount = 1;
    filesize_t size = 0;
    const ClusterType& ptrType() const;

    const static MemberType invalid;

TC_DATABASE_PRIVATE:
    ClusterType* target_type = nullptr;
};

class ClusterType
{
public:
    std::string name;
    std::vector<MemberType> members;
    filesize_t size = 0;

    bool operator==(const ClusterType&) const;
    bool operator!=(const ClusterType&) const;
    
    bool contains(const std::string&) const;
    MemberType& getMember(const std::string&);
    const MemberType& getMember(const std::string&) const;

    const static ClusterType invalid;
};

} // namespace database_detail
} // namespace database_datail

#endif