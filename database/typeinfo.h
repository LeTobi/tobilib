#ifndef TC_DATABASE_TYPEINFO_H
#define TC_DATABASE_TYPEINFO_H

#include <string>
#include <vector>
#include "../stringplus/filename.h"
#include "../general/exception.hpp"

namespace tobilib {
namespace database {

	struct BlockInfo;
	struct ClusterInfo;
	struct DatabaseInfo;

    enum class BlockType {
		t_int,
		t_char,
		t_decimal,
		t_bool,
		t_cluster_ptr,
		t_list_ptr,
	};

	struct BlockInfo
	{
		std::string name;
		BlockType type;
		unsigned int amount;
		ClusterInfo* ptr_type;
	};

	struct ClusterInfo
	{
		std::string name;
		std::vector<BlockInfo> segmentation;
		FileName filename;

		const BlockInfo* find(const std::string&) const;
		BlockInfo* find(const std::string&);
	};

	struct DatabaseInfo
	{
		FileName path;
		std::vector<ClusterInfo> types;

		const ClusterInfo* find(const std::string&) const;
		ClusterInfo* find(const std::string&);
	};

} // namespace Database
} // namespace tobilib

#endif