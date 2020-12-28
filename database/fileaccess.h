#ifndef TC_DATABASE_FILEACCESS_H
#define TC_DATABASE_FILEACCESS_H

#include "concepts.h"
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
    using LineIndex = unsigned int;

    ListFile(Database*);

    void open();

    LineIndex get_index(std::streampos position) const;
    LineIndex capacity() const;

    // single line access
    LineIndex get_first_empty() const;
    LineIndex get_last_empty() const;
    LineIndex get_next(LineIndex) const;
    LineIndex get_previous(LineIndex) const;
    std::streampos data_location(LineIndex) const;
    void set_first_empty(LineIndex);
    void set_last_empty(LineIndex);
    void set_next(LineIndex,LineIndex);
    void set_previous(LineIndex,LineIndex);
    LineIndex extend();

    // leaking modification
    LineIndex remove_empty();
    void append_empty(LineIndex);
    void append_filled(LineIndex, LineIndex);
    LineIndex remove_filled(LineIndex);

    // stable modification
    LineIndex emplace(LineIndex);
    LineIndex erase(LineIndex);

    const static std::streampos LINESIZE;
    const static std::streampos LINEHEAD;
};

class ClusterFile: public File
{
public:
    using LineIndex = unsigned int;

    ClusterFile(Database*);

    void open();

    // single entry access
    LineIndex get_first_filled() const;
    LineIndex get_first_empty() const;
    LineIndex get_last_filled() const;
    LineIndex get_last_empty() const;
    LineIndex get_next(LineIndex) const;
    LineIndex get_previous(LineIndex) const;
    bool get_occupied(LineIndex) const;
    unsigned int get_refcount(LineIndex) const;
    LineIndex capacity() const;
    std::streampos data_location(LineIndex) const;
    void set_first_filled(LineIndex);
    void set_first_empty(LineIndex);
    void set_last_filled(LineIndex);
    void set_last_empty(LineIndex);
    void set_next(LineIndex,LineIndex);
    void set_previous(LineIndex, LineIndex);
    void set_occupied(LineIndex, bool);
    void clear_refcount(LineIndex);
    void set_refcount_add(LineIndex, int);
    unsigned int extend();

    // leaking modification
    LineIndex remove_empty();
    void append_empty(LineIndex);
    void append_filled(LineIndex);
    void remove_filled(LineIndex);

    // stable modification
    LineIndex emplace();
    void erase(LineIndex);

    ClusterType type;

    std::streampos linesize() const;
    const static std::streampos LINEHEAD;
};

} // namespace database
} // namespace tobilib

#endif