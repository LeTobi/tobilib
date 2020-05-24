#ifndef TC_DATABASE_H
#define TC_DATABASE_H

#include <fstream>
#include <vector>
#include <list>
#include <map>
#include "../stringplus/filename.h"
#include "../general/exception.hpp"
#include <memory> // unique_ptr for operator ->

namespace tobilib 
{

class Database
{
private:
	class File;
	class ClusterFile;

public:
	mutable Logger log = std::string("database: ");

	class BlockType;
	class MemberType;
	class ClusterType;
	class Element;
	class Iterator;
	class const_Iterator;

	enum class Status
	{
		empty,
		closed,
		open,
		error
	};

	class BlockType
	{
	public:
		std::streampos size;

		BlockType(): id(0), size(0) {};

		bool operator==(const BlockType& other) const {return other.id==id;};
		bool operator!=(const BlockType& other) const {return !(*this==other);};

		const static BlockType t_int;
		const static BlockType t_char;
		const static BlockType t_double;
		const static BlockType t_bool;
		const static BlockType t_list;
		const static BlockType t_ptr;

	private:
		unsigned int id;
		BlockType(unsigned int _id, std::streampos _size): id(_id), size(_size) {};
	};

	class MemberType {
	public:
		BlockType blockType;
		unsigned int amount;
		ClusterType* ptr_type = nullptr;

		std::streampos size() const;
	};

	class ClusterType
	{
	public:
		std::string name;
		std::map<std::string,MemberType> members;

		bool operator==(const ClusterType&) const;
		bool operator!=(const ClusterType&) const;
		std::streampos size() const;
		std::streampos offsetOf(const std::string&) const;
	};

	class Element
	{
	public:
		enum class Type
		{
			cluster,
			clusterList,
			block,
			array,
			list,
			ptr,
			null
		};

		Element();
		void copy(const Element&);

		bool is_null() const;
		bool operator==(const Element&) const;
		bool operator!=(const Element&) const;

		operator int() const;
		operator char() const;
		operator std::string() const;
		operator double() const;
		operator bool() const;
		Element operator*() const;
		std::unique_ptr<Element> operator->() const;

		void operator=(int);
		void operator=(char);
		void operator=(const std::string&);
		void operator=(double);
		void operator=(bool);
		void operator=(const Element&);

		Element operator()(const std::string&) const;
		unsigned int index() const;
		void erase();
		unsigned int reference_count();
		void clear_references();

		unsigned int size() const;
		Element operator[](unsigned int);
		const Element operator[](unsigned int) const;
		Iterator begin();
		Iterator end();
		const_Iterator begin() const;
		const_Iterator end() const;
		const_Iterator cbegin() const;
		const_Iterator cend() const;
		Element emplace();
		void erase(const const_Iterator&);

		const static Element null;

	private:
		friend class Database;
		friend class Iterator;

		inline ClusterType* clusterType();
		inline const ClusterType* clusterType() const;
		inline ClusterFile* clusterFile();
		inline const ClusterFile* clusterFile() const;
		void add_refcount(int);
		void cluster_init();

		Type type;
		File* fs;
		std::streampos position;
		Database* database;
		
		MemberType memberType;
	};

	class Iterator
	{
	public:
		Iterator& operator++();
		Iterator operator++(int);

		bool operator==(const Iterator&) const;
		bool operator!=(const Iterator&) const;

		Element operator*() const;
		Element* operator->() const;

	private:
		friend class Element;
		mutable Element ref = Element::null;
		Element parent = Element::null;
	};

	class const_Iterator
	{
	private:
		friend class Element;
		Iterator it;
	public:
		const_Iterator() {};
		const_Iterator(const Iterator& other): it(other) {};
		const_Iterator& operator++() {it++; return *this;};
		const_Iterator operator++(int){return it++;};
		bool operator==(const const_Iterator& other) const {return it==other.it;};
		bool operator!=(const const_Iterator& other) const {return it!=other.it;};
		const Element operator*() const {return *it;};
		const Element operator->() const {return *it;};
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
	Element list(const std::string&);
	const Element list(const std::string&) const;

private:
	friend class Element;
	friend class Iterator;

	class File
	{
	public:
		Database* database;
		FileName name;
		mutable std::fstream fs;

		File(Database*);

		void open();
		void close();
		std::streampos size() const;

		template<class PrimT>
		inline PrimT readAt(std::streampos) const;
		template<class PrimT>
		inline void writeAt(std::streampos,PrimT);
	};

	class ClusterFile: public File
	{
	public:
		ClusterFile(Database*);

		void open();

		// raw access
		std::streampos get_first_filled() const;
		std::streampos get_first_empty() const;
		std::streampos get_last_filled() const;
		std::streampos get_last_empty() const;
		std::streampos get_next(std::streampos) const;
		std::streampos get_previous(std::streampos) const;
		bool get_occupied(std::streampos) const;
		unsigned int get_refcount(std::streampos) const;
		void set_first_filled(std::streampos);
		void set_first_empty(std::streampos);
		void set_last_filled(std::streampos);
		void set_last_empty(std::streampos);
		void set_next(std::streampos,std::streampos);
		void set_previous(std::streampos,std::streampos);
		void set_occupied(std::streampos, bool);
		void clear_refcount(std::streampos);
		void set_refcount_add(std::streampos, int);
		std::streampos extend();

		// link control
		std::streampos remove_empty();
		void append_empty(std::streampos);
		void append_filled(std::streampos);
		void remove_filled(std::streampos);

		// space control
		std::streampos emplace();
		void erase(std::streampos);

		// content control
		Element list();
		const Element list() const;
		Element at(std::streampos);

		ClusterType info;
	};

	class ListFile: public File
	{
	public:
		ListFile(Database*);

		void open();

		// raw access
		std::streampos get_first_empty() const;
		std::streampos get_last_empty() const;
		std::streampos get_next(std::streampos) const;
		std::streampos get_previous(std::streampos) const;
		void set_first_empty(std::streampos);
		void set_last_empty(std::streampos);
		void set_next(std::streampos,std::streampos);
		void set_previous(std::streampos,std::streampos);
		std::streampos extend();

		// link control
		std::streampos remove_empty();
		void append_empty(std::streampos);

		std::streampos ptr_pos(std::streampos) const;

		const static std::streampos LINESIZE;
	};

	FileName path;
	mutable Status status;
	ListFile listfile;
	File structurefile;
	std::list<ClusterFile> clusters;
	const static std::string valid_chars;

	ClusterFile* get_cluster(const std::string&);
	ClusterFile* get_cluster(const ClusterType*);

	static bool valid_name(const std::string&);
	void parse_structure();
	bool parse_check(const std::string&);
	void parse_typenames();
	void parse_cluster();
	void parse_block(ClusterType&);
	void parse_arr_len(unsigned int&);
	void parse_errorlog(const std::string&);

};

} // namespace tobilib

#endif