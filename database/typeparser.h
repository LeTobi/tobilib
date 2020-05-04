#ifndef TC_DATABASE_TYPEPARSER_H
#define TC_DATABASE_TYPEPARSER_H

#include "../general/exception.hpp"
#include "../stringplus/filename.h"
#include "typeinfo.h"
#include <fstream>

namespace tobilib {
namespace database {

class TypeParser
{
public:
    enum class Result {
        success,
        file_access_error,
        format_error
    };

    Logger log = std::string("database::TypeParser: ");
    Result result;
    FileName path;

    const static std::string valid_chars;
    static bool valid_name(const std::string&);
    bool parse(DatabaseInfo&);
    bool write(const DatabaseInfo&);

private:
    std::fstream fs;
    bool parse_all_typenames(DatabaseInfo&);
    bool parse_cluster(DatabaseInfo&,ClusterInfo&);
    bool parse_block(DatabaseInfo&,BlockInfo&);
    bool parse_arr_len(unsigned int&);
    void write_primitive(const BlockInfo&);
    
};

} // namespace database
} // namespace tobilib

#endif