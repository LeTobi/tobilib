#ifndef TC_DATABASE_COMMANDS_H
#define TC_DATABASE_COMMANDS_H

#include "../database.h"
#include <sstream>

namespace tobilib {
namespace database_tools{

std::string command(Database& db, const std::string);

namespace detail {

    enum class TargetType {
        clusterlist,
        cluster,
        primitiveMember,
        array,
        pointer,
        list,
        nullpointer,
        invalid
    };

    struct Target {
        TargetType type = TargetType::invalid;
        Database::ClusterList list;
        Database::Cluster cluster;
        Database::Member member;
    };

    Target resolve(Database&, std::istream&);
    Target resolve(Target, std::vector<StringPlus>&);
    Target select_type(Database::Member);
    bool set(Database&, Target, std::istream&);
    std::string print(Target);

} // namespace detail
} // namespace database_tools
} // namespace tobilib

#endif