#ifndef TC_DATABASE_FILEIO_H
#define TC_DATABASE_FILEIO_H

#include "concepts.h"

namespace tobilib {
namespace database_detail {

template<class PrimT>
filesize_t serial_size();

class File : public Component
{
public:
    FileName name;

    File(Database*);
    File(const File&) = delete;
    ~File();

    void open();
    void close();
    filesize_t size() const;
    void extend(filesize_t);
    void save() const;
    bool is_locked(off_t start, off_t len);
    bool lock(off_t start, off_t len);
    void unlock(off_t start, off_t len);

    std::string readSomeAt(filesize_t,filesize_t) const;
    void writeSomeAt(filesize_t,const std::string&);

    template<class PrimT>
    PrimT readAt(filesize_t) const;
    template<class PrimT>
    void writeAt(filesize_t, PrimT);

private:
    bool finish_kernel(int) const;
    mutable off_t kernel_result;
    mutable int kernel_error;

    bool is_open = false;
    int descriptor; // this is actually a file *description* and not file *descriptor*

    const static mode_t FILEMODE;
};

} // namespace database_detail
} // namespace tobilib

#endif