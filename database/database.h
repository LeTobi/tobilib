#ifndef TC_DATABASE_H
#define TC_DATABASE_H

#include <fstream>
#include <map>
#include "../stringplus/filename.h"
#include "../general/exception.hpp"
#include "typeinfo.h"
#include "element.h"
#include "typeparser.h"

namespace tobilib 
{
namespace database
{

class Access
{
public:
	mutable Logger log = std::string("database::Access: ");
	
	Element list(const std::string&);
	
	bool init(const FileName&);
	bool init(const DatabaseInfo&);
	bool is_init() const;
	bool open();
	bool is_good() const;
	bool is_open() const;
	void close();
	void clear();

private:
	friend class Element;

	struct File {
		FileName name;
		mutable std::fstream stream;

		unsigned int size() const;
	};

	enum class State {
		empty,
		closed,
		open,
		error,
	};

	mutable State state = State::empty;
	DatabaseInfo typeinfo;
	File listfile;
	std::map<ClusterInfo*,File> files;

	bool openFile(File&);

	const static unsigned int listItemSize;
	static std::streampos clusterSize(const ClusterInfo&);
	static std::streampos blockSize(const BlockInfo&);
	static std::streampos primitiveSize(const BlockInfo&);
	unsigned int read_index(std::fstream&) const;
	void write_index(std::fstream&, unsigned int);
	void initialize(const ClusterInfo&, unsigned int);
}; // class Access

} // namespace database
} // namespace tobilib

#include "iterator.h"

#endif