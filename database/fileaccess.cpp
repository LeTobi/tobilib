#define TC_DATABASE_INTERN

#include "fileaccess.h"
#include "database.h"
#include <cmath>

using namespace tobilib;
using namespace database_detail;

template<class PrimT>
filesize_t database_detail::serial_size()
{
    return sizeof(PrimT);
}

template<bool>
filesize_t serial_size()
{
    return sizeof(char);
}

template filesize_t database_detail::serial_size<int>();
template filesize_t database_detail::serial_size<unsigned int>();
template filesize_t database_detail::serial_size<double>();
template filesize_t database_detail::serial_size<char>();

template<class PrimT>
bool serial_print(std::fstream& fs, PrimT out)
{
    char* data = reinterpret_cast<char*>(&out);
    for (int i=0;i<sizeof(PrimT);i++) {
        fs.put(data[i]);
    }
    return fs.good();
}

template<bool>
bool serial_print(std::fstream& fs, bool out)
{
    fs << out?'1':'0';
    return fs.good();
}

template<class PrimT>
bool serial_read(std::fstream& fs, PrimT& in) {
    char* data = reinterpret_cast<char*>(&in);
    for (int i=0;i<sizeof(PrimT);i++) {
        data[i] = fs.get();
    }
    return fs.good();
}

template<bool>
bool serial_read(std::fstream& fs, bool& in) {
    in = fs.get()=='1';
    return fs.good();
}

File::File(Database* db): Component(db)
{ }

void File::open()
{
    if (!pre_good())
        return;
    fs.close();
    fs.open(name.fullName(),fs.in|fs.out);
    if (fs.good())
        return;
    fs.close();
    fs.open(name.fullName(),fs.out);
    if (!fs.good()) {
        database->status = Database::Status::error;
        database->log << name.fullName() << " konnte nicht geoeffnet werden" << std::endl;
        return;
    }
    database->log << name.fullName() << " wurde neu erstellt" << std::endl;
    fs.close();
    fs.open(name.fullName(),fs.in|fs.out);
    if (!fs.good())
    {
        database->status = Database::Status::error;
        database->log << name.fullName() << " kann nicht gelesen werden" << std::endl;
        return;
    }
}

void File::close()
{
    fs.close();
}

filesize_t File::size() const
{
    if (!pre_good())
        return 0;
    fs.seekg(0,fs.end);
    filesize_t len = fs.tellg();
    if (!fs.good()) {
        database->status = Database::Status::error;
        database->log << "Fehler bei Ermittlung der Dateigroesse: (" << name.fileOnly() << ")" << std::endl;
        return 0;
    }
    return len;
}

void File::extend(filesize_t len)
{
    if (!pre_good())
        return;
    fs.seekp(0,fs.end);
    fs << std::string(len,0);
    if (!fs.good())
    {
        database->status = Database::Status::error;
        database->log << "Fehler beim erweitern der Datei " << name.fileOnly() << std::endl;
        return;
    }
}

template<class PrimT>
PrimT File::readAt(filesize_t where) const
{
    if (!pre_good())
        return 0;
    fs.seekg(where);
    PrimT out;
    if (!serial_read(fs,out)) {
        database->status = Database::Status::error;
        database->log << "Lesefehler an Position " << fs.tellg() << " (" << name.fileOnly() << ")" << std::endl;
        return 0;
    }
    return out;
}

template <class PrimT>
void File::writeAt(filesize_t where, PrimT what)
{
    if (!pre_good())
        return;
    fs.seekp(where);
    if (!serial_print(fs,what)) {
        database->status = Database::Status::error;
        database->log << "Schreibfehler an Position " << fs.tellp() << " (" << name.fileOnly() << ")" << std::endl;
    }
}

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

    restore_all();
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
    
    datafile.fs.flush();
    if (!datafile.fs.good())
    {
        database->status = Database::Status::error;
        database->log << "Backup system: Fehler in der Datendatei " << datafile.name.fileOnly() << std::endl;
        return;
    }

    for (unsigned int i = 0; i<chunks.size(); i++)
        set_backup_flag(chunks.size()-1-i,false);
    backupfile.fs.flush();
    if (!backupfile.fs.good())
    {
        database->status = Database::Status::error;
        database->log << "Die Backupdatei konnte nicht zurueckgesetzt werden. " << backupfile.name.fileOnly() << std::endl;
        return;
    }
    chunks.clear();
}

void CrashSafeFile::restore_all()
{
    unsigned int backup_idx;
    unsigned int backup_count = 0;
    for (backup_idx=0;backup_idx<get_backup_capacity();backup_idx++)
    {
        if (!get_backup_flag(backup_idx))
            return;
        ++backup_count;
        filesize_t length = prepare_data_transfer(backup_idx);
        if (!pre_good())
            return;
        for (unsigned int j=0;j<length;j++)
            datafile.fs.put(backupfile.fs.get());
    }
    datafile.fs.flush();
    if (!datafile.fs.good() || !backupfile.fs.good())
    {
        database->status = Database::Status::error;
        database->log << "Daten konnten nicht wieder hergestellt werden. " << backupfile.name.fileOnly() << std::endl;
        return;
    }
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
    filesize_t length = prepare_data_transfer(backup_idx);

    if (!pre_good())
        return;
    for (unsigned int i=0;i<length;i++)
        backupfile.fs.put(datafile.fs.get());
    backupfile.fs.flush();
    if (!datafile.fs.good() || !backupfile.fs.good())
    {
        database->status = Database::Status::error;
        database->log << "Es konnte kein Backup von den Daten genommen werden. " << datafile.name.fileOnly() << std::endl;
        return;
    }

    set_backup_flag(backup_idx, true);
    backupfile.fs.flush();
    if (!backupfile.fs.good())
    {
        database->status = Database::Status::error;
        database->log << "Es konnte kein Backup von den Daten genommen werden. " << backupfile.name.fileOnly() << std::endl;
    }

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
    std::streamsize pos = where*BACKUP_CHUNKSIZE;
    pos += serial_size<bool>();
    DataIndex data_idx = backupfile.readAt<DataIndex>(pos);
    return data_idx * DATA_CHUNKSIZE;
}

filesize_t CrashSafeFile::prepare_data_transfer(BackupIndex where)
{
    filesize_t datapos = get_data_stream_offset(where);
    filesize_t backuppos = get_backup_stream_offset(where);
    filesize_t datalen = std::min(
        DATA_CHUNKSIZE,
        filesize_t(datafile.size() - datapos)
    );
    datafile.fs.seekg(datapos);
    datafile.fs.seekp(datapos);
    backupfile.fs.seekg(backuppos);
    backupfile.fs.seekp(backuppos);
    return datalen;
}

void CrashSafeFile::set_data_index(BackupIndex where, DataIndex what)
{
    std::streamsize pos = where*BACKUP_CHUNKSIZE;
    pos += serial_size<bool>();
    backupfile.writeAt(pos,what);
}

filesize_t CrashSafeFile::get_backup_stream_offset(BackupIndex where)
{
    std::streamsize pos = where*BACKUP_CHUNKSIZE;
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