#ifndef TC_DATABASE_PARSER_H
#define TC_DATABASE_PARSER_H

#include "concepts.h"
#include "fileaccess.h"

namespace tobilib {
namespace database_detail {

class Parser: public Component
{
public:
	using File = database_detail::File;

    Parser(Database*);

	void parse_all();

TC_DATABASE_PRIVATE:
    const static std::string valid_chars;
	File structurefile;

    static bool valid_name(const std::string&);
	bool next_matches(const std::string&);
	void parse_typenames();
	void parse_cluster();
	void parse_block(ClusterType&);
	void parse_arr_len(unsigned int&);
	void errorlog(const std::string&);
};

} // namespace database_detail
} // namespace tobilib

#endif