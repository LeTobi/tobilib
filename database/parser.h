#ifndef TC_DATABASE_PARSER_H
#define TC_DATABASE_PARSER_H

#include "concepts.h"
#include "../general/exception.hpp"
#include <fstream>

namespace tobilib {
namespace database_detail {

class Parser: public Component
{
public:
    Parser(Database*);

	void parse_all();

TC_DATABASE_PRIVATE:
    const static std::string valid_chars;
	std::fstream file;
	Logger log;

    static bool valid_name(const std::string&);
	bool next_matches(const std::string&);
	bool is_eof();
	void parse_typenames();
	void parse_cluster();
	void parse_block(ClusterType&);
	void parse_arr_len(unsigned int&);
	void errorlog(const std::string&);
};

} // namespace database_detail
} // namespace tobilib

#endif