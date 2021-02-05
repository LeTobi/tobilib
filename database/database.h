#ifndef TC_DATABASE_H
#define TC_DATABASE_H

#include <list>
#include "concepts.h"
#include "type.h"
#include "member.h"
#include "cluster.h"
#include "fileaccess.h"
#include "../general/exception.hpp"
#include "../general/requestflag.h"

namespace tobilib 
{

class Database
{
public:
	mutable Logger log = std::string("database: ");

	using BlockType = database_detail::BlockType;
	using MemberType = database_detail::MemberType;
	using ClusterType = database_detail::ClusterType;
	using Member = database_detail::Member;
	using ClusterList = database_detail::ClusterList;
	using Cluster = database_detail::Cluster;
	using MemberIterator = database_detail::MemberIterator;
	using clusterIterator = database_detail::ClusterIterator;

	enum class Status
	{
		empty,
		closed,
		open,
		error
	};

	Database();
	Database(const FileName&);

	void setPath(const FileName&);
	bool init();
	bool is_init() const;
	bool open();
	bool is_open() const;
	void close();
	void clear();
	bool is_good() const;
	ClusterList list(const std::string&);
	const ClusterList list(const std::string&) const;

	FlagRequest begin_critical_operation();
	bool critical_operation_running() const;
	void end_critical_operation(FlagRequest);

TC_DATABASE_PRIVATE:
	using Component = database_detail::Component;
	using ClusterFile = database_detail::ClusterFile;
	using ListFile = database_detail::ListFile;

	FileName path;
	mutable Status status;
	ListFile listfile;
	std::list<ClusterFile> clusters;
	RequestFlag critical_operation;

	ClusterFile* get_file(const std::string&);
	ClusterFile* get_file(const ClusterType*);

};

} // namespace tobilib

#endif