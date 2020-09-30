#ifndef TC_DATABASE_FILEACCESS_H
#define TC_DATABASE_FILEACCESS_H

#include "concepts.h"
#include "member.h"
#include <fstream>

namespace tobilib {
namespace database_detail {

template<class PrimT>
std::streampos serial_size();

class File : public Component
{
public:
    FileName name;
    mutable std::fstream fs;

    File(Database*);

    void open();
    void close();
    std::streampos size() const;

    template<class PrimT>
    PrimT readAt(std::streampos) const;
    template<class PrimT>
    void writeAt(std::streampos,PrimT);
};

class ListFile: public File
{
public:
    ListFile(Database*);

    void open();

    Member create_member(const Member&, unsigned int) const;
    unsigned int get_index(const Member&) const;

    // element access
    unsigned int get_first_empty() const;
    unsigned int get_last_empty() const;
    unsigned int get_next(unsigned int) const;
    unsigned int get_previous(unsigned int) const;
    unsigned int get_first_filled(const Member&) const;
    void set_first_empty(unsigned int);
    void set_last_empty(unsigned int);
    void set_next(unsigned int,unsigned int);
    void set_previous(unsigned int,unsigned int);
    void set_first_filled(const Member&,unsigned int);
    unsigned int extend();

    // list access
    unsigned int remove_empty();
    void append_empty(unsigned int);
    void append_filled(const Member&, unsigned int);
    void remove_filled(const Member&, unsigned int);

    // file access
    Member emplace(const Member&);
    void erase(const Member&, const Member&);
    Member begin(const Member&);
    Member next(const Member&, const Member&);
    const Member begin(const Member&) const;
    const Member next(const Member&, const Member&) const;


    const static std::streampos LINESIZE;
    const static std::streampos LINEHEAD;
};

class ClusterFile: public File
{
public:
    ClusterFile(Database*);

    void open();

    Cluster create_cluster(unsigned int) const;
    unsigned int get_index(const Cluster&) const;

    // element access
    unsigned int get_first_filled() const;
    unsigned int get_first_empty() const;
    unsigned int get_last_filled() const;
    unsigned int get_last_empty() const;
    unsigned int get_next(unsigned int) const;
    unsigned int get_previous(unsigned int) const;
    bool get_occupied(unsigned int) const;
    unsigned int get_refcount(unsigned int) const;
    void set_first_filled(unsigned int);
    void set_first_empty(unsigned int);
    void set_last_filled(unsigned int);
    void set_last_empty(unsigned int);
    void set_next(unsigned int,unsigned int);
    void set_previous(unsigned int, unsigned int);
    void set_occupied(unsigned int, bool);
    void clear_refcount(unsigned int);
    void set_refcount_add(unsigned int, int);
    unsigned int extend();

    // list access
    unsigned int remove_empty();
    void append_empty(unsigned int);
    void append_filled(unsigned int);
    void remove_filled(unsigned int);

    // file access
    Cluster emplace();
    void erase(const Cluster&);
    Cluster at(unsigned int);
    Cluster next(const Cluster&);
    Cluster begin();
    const Cluster at(unsigned int) const;
    const Cluster next(const Cluster&) const;
    const Cluster begin() const;

    unsigned int refcount(const Cluster&);
    void add_refcount(const Cluster&, int);
    bool is_occupied(const Cluster&);

    ClusterType type;

    std::streampos linesize() const;
    const static std::streampos LINEHEAD;
};

} // namespace database
} // namespace tobilib

#endif