#ifndef TC_DATABASE_EXPORT_H
#define TC_DATABASE_EXPORT_H

#include "../database.h"
#include <iostream>

namespace tobilib {
namespace database_tools {

    struct Result {
        Result() { };
        Result(const char* msg): good(false), info(msg) { };

        bool good = true;
        std::string info;

        operator bool(){ return good; };
    };

    Result export_database(const Database&);
    Result export_database(const Database&, const FileName&);

namespace detail {

    #ifdef TC_DATABASE_INTERN
        Result print_listfile(const Database::ListFile&, const FileName&);
        void print_member(const Database::Member&, std::ostream&);
        Result print_clusterfile(const Database::ClusterFile&, const FileName&);
    #endif

} // namespace detail
} // namespace database_tools
} // namespace tobilib

#endif