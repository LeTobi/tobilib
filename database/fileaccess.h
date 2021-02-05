#ifndef TC_DATABASE_FILEACCESS_H
#define TC_DATABASE_FILEACCESS_H

#include "concepts.h"
#include <fstream>
#include <set>

namespace tobilib {
namespace database_detail {

template<class PrimT>
filesize_t serial_size();

class File : public Component
{
public:
    FileName name;
    mutable std::fstream fs;

    File(Database*);
    File(const File&) = delete;

    void open();
    void close();
    filesize_t size() const;
    void extend(filesize_t);

    template<class PrimT>
    PrimT readAt(filesize_t) const;
    template<class PrimT>
    void writeAt(filesize_t, PrimT);
};

class CrashSafeFile : public Component
{
public:
    FileName name;

    CrashSafeFile(Database*);

    void open();
    void close();
    filesize_t size() const;
    void extend(filesize_t);
    
    template<class PrimT>
    PrimT readAt(filesize_t) const;
    template<class PrimT>
    void writeAt(filesize_t, PrimT);

    void confirm();
    
private:
    File datafile;
    File backupfile;

    using BackupIndex = unsigned int;
    using DataIndex = unsigned int;

    void restore_all();
    void backup(filesize_t);

    bool get_backup_flag(BackupIndex);
    void set_backup_flag(BackupIndex, bool);
    filesize_t get_data_stream_offset(BackupIndex);
    filesize_t prepare_data_transfer(BackupIndex);
    void set_data_index(BackupIndex, DataIndex);
    filesize_t get_backup_stream_offset(BackupIndex);
    DataIndex get_data_chunk_index(filesize_t);
    BackupIndex get_backup_capacity();
    void extend_backup_capacity();

    std::set<DataIndex> chunks;
    bool file_opened = false;

    const static filesize_t DATA_CHUNKSIZE;
    const static filesize_t BACKUP_CHUNKSIZE;
};

} // namespace database
} // namespace tobilib

#endif