#define TC_DATABASE_INTERN

#include "filebackup.h"
#include "database.h"
#include <cmath>

using namespace tobilib;
using namespace database_detail;

CrashSafeFile::CrashSafeFile(Database* db):
    Component(db),
    datafile(db),
    backupfile(db)
{ }

void CrashSafeFile::open()
{
    if (file_opened)
        throw Exception("Die Datei sollte nicht doppelt geoeffnet werden","CrashSafeFile::open()");
    FileName tmpdir = name;
    datafile.name = name;
    datafile.name.extension = "data";
    datafile.open();
    backupfile.name = name;
    backupfile.name.extension = "tmp";
    backupfile.open();

    if (!pre_good())
    {
        datafile.close();
        backupfile.close();
        return;
    }

    if (backupfile.size()%BACKUP_CHUNKSIZE != 0)
    {
        database->log << "Die Groesse von " << backupfile.name.fileOnly() << " ist fehlerhaft und wird korrigiert" << std::endl;
        extend_backup_capacity();
    }

    if (pre_good())
    {
        file_opened = true;
    }
    else
    {
        datafile.close();
        backupfile.close();
        return;
    }
}

void CrashSafeFile::close()
{
    file_opened = false;
    datafile.close();
    backupfile.close();
}

filesize_t CrashSafeFile::size() const
{
    if (!pre_good())
        return 0;
    if (!file_opened)
        throw Exception("Datei nicht offen","CrashSafeFile::size()");
    return datafile.size();
}

void CrashSafeFile::extend(filesize_t len)
{
    if (!pre_good())
        return;
    if (!file_opened)
        throw Exception("Datei nicht offen","CrashSafeFile::extend()");
    datafile.extend(len);
}

template<class PrimT>
PrimT CrashSafeFile::readAt(filesize_t where) const
{
    return datafile.readAt<PrimT>(where);
}

template int          CrashSafeFile::readAt(filesize_t) const;
template unsigned int CrashSafeFile::readAt(filesize_t) const;
template double       CrashSafeFile::readAt(filesize_t) const;
template char         CrashSafeFile::readAt(filesize_t) const;
template bool         CrashSafeFile::readAt(filesize_t) const;

template<class PrimT>
void CrashSafeFile::writeAt(filesize_t where, PrimT what)
{
    if (!pre_good())
        return;
    if (database->critical_operation_running())
    {
        backup(where);
        backup(where + serial_size<PrimT>() - 1);
        if (!pre_good())
            return;
    }
    datafile.writeAt<PrimT>(where,what);
}

template void CrashSafeFile::writeAt(filesize_t,int);
template void CrashSafeFile::writeAt(filesize_t,unsigned int);
template void CrashSafeFile::writeAt(filesize_t,double);
template void CrashSafeFile::writeAt(filesize_t,char);
template void CrashSafeFile::writeAt(filesize_t,bool);

void CrashSafeFile::confirm()
{
    if (!pre_good() || !file_opened)
        return;
    
    datafile.save();
    for (unsigned int i = 0; i<chunks.size(); i++)
        set_backup_flag(chunks.size()-1-i,false);
    backupfile.save();
    chunks.clear();
}

void CrashSafeFile::restore()
{
    unsigned int backup_idx;
    unsigned int backup_count = 0;
    for (backup_idx=0;backup_idx<get_backup_capacity();backup_idx++)
    {
        if (!get_backup_flag(backup_idx))
            break;
        ++backup_count;
        filesize_t datapos = get_data_stream_offset(backup_idx);
        filesize_t backuppos = get_backup_stream_offset(backup_idx);
        datafile.writeSomeAt(datapos, backupfile.readSomeAt(backuppos, DATA_CHUNKSIZE));
    }
    datafile.save();
    if (!pre_good())
        return;
    if (backup_count>0)
        database->log << backup_count << " chunks von " << datafile.name.fileOnly() << " wiederhergestellt." << std::endl;
    confirm();
}

void CrashSafeFile::backup(filesize_t what)
{
    DataIndex data_idx = get_data_chunk_index(what);
    if (chunks.count(data_idx)>0)
        return;
    BackupIndex backup_idx = chunks.size();
    if (backup_idx>=get_backup_capacity())
        extend_backup_capacity();
    set_backup_flag(backup_idx,false);
    set_data_index(backup_idx,data_idx);

    filesize_t datapos = get_data_stream_offset(backup_idx);
    filesize_t backuppos = get_backup_stream_offset(backup_idx);
    backupfile.writeSomeAt(backuppos, datafile.readSomeAt(datapos,DATA_CHUNKSIZE));
    backupfile.save();

    set_backup_flag(backup_idx, true);
    backupfile.save();

    if (pre_good())
        chunks.insert(data_idx);
}

bool CrashSafeFile::get_backup_flag(BackupIndex where)
{
    std::streamsize pos = where*BACKUP_CHUNKSIZE;
    return backupfile.readAt<bool>(pos);
}

void CrashSafeFile::set_backup_flag(BackupIndex where, bool what)
{
    std::streamsize pos = where*BACKUP_CHUNKSIZE;
    backupfile.writeAt(pos,what);
}

filesize_t CrashSafeFile::get_data_stream_offset(BackupIndex where)
{
    filesize_t pos = where*BACKUP_CHUNKSIZE;
    pos += serial_size<bool>();
    DataIndex data_idx = backupfile.readAt<DataIndex>(pos);
    return data_idx * DATA_CHUNKSIZE;
}

void CrashSafeFile::set_data_index(BackupIndex where, DataIndex what)
{
    filesize_t pos = where*BACKUP_CHUNKSIZE;
    pos += serial_size<bool>();
    backupfile.writeAt(pos,what);
}

filesize_t CrashSafeFile::get_backup_stream_offset(BackupIndex where)
{
    filesize_t pos = where*BACKUP_CHUNKSIZE;
    pos += serial_size<bool>();
    pos += serial_size<DataIndex>();
    return pos;
}

CrashSafeFile::DataIndex CrashSafeFile::get_data_chunk_index(filesize_t where)
{
    return where/DATA_CHUNKSIZE;
}

CrashSafeFile::BackupIndex CrashSafeFile::get_backup_capacity()
{
    return backupfile.size() / BACKUP_CHUNKSIZE;
}

void CrashSafeFile::extend_backup_capacity()
{
    filesize_t overhead = backupfile.size()%BACKUP_CHUNKSIZE;
    BackupIndex idx = get_backup_capacity();
    if (overhead < serial_size<bool>())
        backupfile.extend(serial_size<bool>());
    set_backup_flag(idx,false);
    overhead = backupfile.size()%BACKUP_CHUNKSIZE;
    backupfile.extend(BACKUP_CHUNKSIZE-overhead);
}

const filesize_t CrashSafeFile::DATA_CHUNKSIZE = 200;
const filesize_t CrashSafeFile::BACKUP_CHUNKSIZE = DATA_CHUNKSIZE + serial_size<bool>() + serial_size<unsigned int>();