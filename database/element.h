/*
	Dieser Header ist Teil von database.h und
	soll nicht manuell eingebunden werden!
*/

#ifndef TC_DATABASE_ELEMENT_H
#define TC_DATABASE_ELEMENT_H

#ifndef TC_DATABASE_H
#error Verwende tobilib/database/database.h
#endif


namespace tobilib {
namespace database {

class Access;
class Iterator;

class Element
{
public:
	enum class Type
	{
		cluster_file,
		cluster,
		array,
		block,
		null
	};

	Element ();

	static Element null;

	Type get_type() const;
	bool is_null() const;

	// array / block:list_ptr / cluster_file
	const Element operator[] (unsigned int) const;
	Element operator[] (unsigned int);

	unsigned int size() const;

	const Iterator begin() const;
	Iterator begin();

	const Iterator end() const;
	Iterator end();

	Element emplace_back();
	void remove(const Element&);
	void insert(const Element&, const Element& successor = Element::null);
	void clear();

	// block:t_int
	operator int() const;
	void operator=(int);

	// block:t_char
	operator char() const;
	void operator=(char);

	// block:t_decimal
	operator double() const;
	void operator=(double);

	// block:t_bool
	operator bool() const;
	void operator=(bool);

	// block:t_cluster_ptr
	const Element operator-> () const;
	const Element operator* () const;
	Element operator->();
	Element operator*();

	// cluster
	const Element operator()(const std::string&) const;
	Element operator()(const std::string&);
	unsigned int index() const;
	bool is_free() const;

	// array mit t_char
	operator std::string() const;
	void operator=(const std::string&);
	void operator=(const char*);

	bool operator== (const Element&) const;

private:
	friend class Access;
	friend class Iterator;

	Type type;
	Access* database;
	std::streampos position;
	std::fstream* fs;

	union {
		ClusterInfo* cluster_file_info;
		ClusterInfo* cluster_info;
		BlockInfo* block_info;
		BlockInfo* array_info;
	};

	Element (Type,Access*);

	void setError(const std::string&) const;
	bool sanityCheck() const;
};

} // namespace database
} // namespace tobilib

#endif