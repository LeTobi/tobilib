#ifndef TC_DATABASE_FILEBACKUP_H
#define TC_DATABASE_FILEBACKUP_H

#include "concepts.h"
#include "fileio.h"
#include <fstream>
#include <set>

namespace tobilib {
namespace database_detail {

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

    // erklärt alle backup Dateien für ungültig.
    void confirm();

    // Stellt den Zustand der Datei wieder her.
    void restore();

private:
    File datafile;
    File backupfile;

    using BackupIndex = unsigned int;
    using DataIndex = unsigned int;

    // erstellt falls nicht schon vorhanden einen Wiederherstellungspunkt für einen Bereich um das angegebene Byte.
    void backup(filesize_t);

    // prüft ob der Backup-Eintrag gültig ist.
    bool get_backup_flag(BackupIndex);

    // bestimmt, ob Backup-Eintrag gültig sein soll.
    void set_backup_flag(BackupIndex, bool);

    // Prüft die Zielposition, wo dieser Backup Eintrag hingehört.
    filesize_t get_data_stream_offset(BackupIndex);

    // Bestimmt die Zielposition, wo dieser Backup Eintrag hingehört.
    void set_data_index(BackupIndex, DataIndex);

    // Prüft, an welcher stelle der Backup Eintrag in der Backup datei zu finden ist.
    filesize_t get_backup_stream_offset(BackupIndex);

    // Bestimmt, zu welchem Dateibereich ein Byte in der Daten-Datei gehört.
    DataIndex get_data_chunk_index(filesize_t);

    // Bestimmt, wie viele Backup-Einträge in die Backupdatei passen.
    BackupIndex get_backup_capacity();

    // Erhöht die Kapazität für Backup-Einträge
    void extend_backup_capacity();

    // Speichert, welche Daten-Bereiche durch Backup gesichert sind
    std::set<DataIndex> chunks;

    bool file_opened = false;

    // Bestimmt die Länge eines Datenbereichs in der Daten-Datei
    const static filesize_t DATA_CHUNKSIZE;

    // Bestimmt die Länge eines Backup-Eintrags mit Meta-Daten in der BackupDatei
    const static filesize_t BACKUP_CHUNKSIZE;
};

} // namespace database
} // namespace tobilib

#endif