#ifndef TC_DATABASE_H
#define TC_DATABASE_H

#include <list>
#include "concepts.h"
#include "type.h"
#include "member.h"
#include "cluster.h"
#include "filetable.h"
#include "filestatus.h"
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
	const ClusterType& getType(const std::string&) const;
	std::vector<std::string> getTypes() const;

	void disable_crash_protection();
	FlagRequest begin_critical_operation();
	bool critical_operation_running() const;
	void end_critical_operation(FlagRequest);

TC_DATABASE_PRIVATE:
	using Component = database_detail::Component;
	using ClusterFile = database_detail::ClusterFile;
	using ListFile = database_detail::ListFile;
	using StatusFile = database_detail::StatusFile;

	FileName path;
	mutable Status status;
	StatusFile statusfile;
	ListFile listfile;
	std::list<ClusterFile> clusters;
	RequestFlag critical_operation;
	bool crash_protection = true;

	ClusterFile* get_file(const std::string&);
	const ClusterFile* get_file(const std::string&) const;
	ClusterFile* get_file(const ClusterType*);
	const ClusterFile* get_file(const ClusterType*) const;

};

} // namespace tobilib

#endif