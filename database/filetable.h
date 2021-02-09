#ifndef TC_DATABASE_FILETABLE_H
#define TC_DATABASE_FILETABLE_H

#include "filebackup.h"
#include "type.h"

namespace tobilib {
namespace database_detail {

class ListFile: public CrashSafeFile
{
    public:
    ListFile(Database*);
    void open();

    LineIndex get_index(filesize_t position) const;

// single line access
    LineIndex get_first_empty() const;
    LineIndex get_last_empty() const;
    LineIndex get_data_capacity() const;
    LineIndex get_next(LineIndex) const;
    LineIndex get_previous(LineIndex) const;
    filesize_t data_location(LineIndex) const;

    TC_DATABASE_PRIVATE:
    void set_first_empty(LineIndex);
    void set_last_empty(LineIndex);
    void set_data_capacity(LineIndex);
    void set_next(LineIndex,LineIndex);
    void set_previous(LineIndex,LineIndex);

// leaking modification
    LineIndex remove_empty();
    void append_empty(LineIndex);
    void append_filled(LineIndex, LineIndex);
    LineIndex remove_filled(LineIndex);
    void extend_data_capacity();

// stable modification
    public:
    LineIndex emplace(LineIndex);
    LineIndex erase(LineIndex);

    const static filesize_t LINESIZE;
    const static filesize_t LINEHEAD;
};

class ClusterFile: public CrashSafeFile
{
    public:
    ClusterFile(Database*);
    void open();

// single entry access
    LineIndex get_first_filled() const;
    LineIndex get_first_empty() const;
    LineIndex get_last_filled() const;
    LineIndex get_last_empty() const;
    LineIndex get_data_capacity() const;
    LineIndex get_next(LineIndex) const;
    LineIndex get_previous(LineIndex) const;
    bool get_occupied(LineIndex) const;
    unsigned int get_refcount(LineIndex) const;
    filesize_t data_location(LineIndex) const;

    TC_DATABASE_PRIVATE:
    void set_refcount_add(LineIndex, int);
    void set_first_filled(LineIndex);
    void set_first_empty(LineIndex);
    void set_last_filled(LineIndex);
    void set_last_empty(LineIndex);
    void set_data_capacity(LineIndex);
    void set_next(LineIndex,LineIndex);
    void set_previous(LineIndex, LineIndex);
    void set_occupied(LineIndex, bool);
    void clear_refcount(LineIndex);

// leaking modification
    LineIndex remove_empty();
    void append_empty(LineIndex);
    void append_filled(LineIndex);
    void remove_filled(LineIndex);
    void extend_data_capacity();

// stable modification
    public:
    LineIndex emplace();
    void erase(LineIndex);

    ClusterType type;

    filesize_t linesize() const;
    const static filesize_t LINEHEAD;
};

} // namespace database_detail
} // namespace tobilib

#endif