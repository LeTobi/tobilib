#ifndef TC_DATABASE_COMMANDS_H
#define TC_DATABASE_COMMANDS_H

#include "../database.h"
#include <sstream>

namespace tobilib {
namespace database_tools{

std::string command(Database& db, const std::string);

namespace detail {

    enum class TargetType {
        none,
        clusterlist,
        cluster,
        primitiveMember,
        array,
        pointer,
        list
    };

    struct Target {
        Target(Database& _db): db(_db) { };

        TargetType type = TargetType::none;
        std::string name;
        Database::Cluster cluster;
        Database::Member member;

        Database& db;
    };

    Target resolve(Database&, std::istream&);
    Target resolve(Target, std::vector<StringPlus>&);
    Target select_type(Database::Member);
    bool set(Target, std::istream&);
    std::string print(Target);

} // namespace detail
} // namespace database_tools
} // namespace tobilib

#endif