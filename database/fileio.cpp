#define TC_DATABASE_INTERN
#include "fileio.h"
#include "database.h"
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <string.h>

using namespace tobilib;
using namespace database_detail;

template<class PrimT>
filesize_t database_detail::serial_size()
{
    return sizeof(PrimT);
}

template filesize_t database_detail::serial_size<int>();
template filesize_t database_detail::serial_size<unsigned int>();
template filesize_t database_detail::serial_size<double>();
template filesize_t database_detail::serial_size<char>();
template filesize_t database_detail::serial_size<bool>();

File::File(Database* db): Component(db)
{ }

File::~File()
{
    close();
}

void File::open()
{
    if (!pre_good())
        return;
    if (is_open)
        return;
    while (finish_kernel(::open(name.fullName().toString().c_str(), O_RDWR | O_CREAT, FILEMODE)));
        // nothing
    if (kernel_result==-1)
    {
        database->status = Database::Status::error;
        database->log << "Fehler beim oeffnen von " << name.fileOnly() << ": " << strerror(kernel_error) << std::endl;
        return;
    }
    is_open=true;
    descriptor=kernel_result;
}

void File::close()
{
    if (!is_open)
        return;
    while(finish_kernel(::close(descriptor)));
        // nothing
    if (kernel_result!=0)
    {
        database->status = Database::Status::error;
        database->log << "Fehler beim schliessen von " << name.fileOnly() << ": " << strerror(kernel_error) << std::endl;
        return;
    }
    is_open = false;
}

filesize_t File::size() const
{
    if (!pre_good())
        return 0;
    if (!is_open)
        throw Exception("Die Datei ist nicht offen","Database::File::size()");
    
    while (finish_kernel(::lseek(descriptor,0,SEEK_END)));
        // nothing
    if (kernel_result == -1)
    {
        database->status = Database::Status::error;
        database->log << "Die Dateigroesse kann nicht ermittelt werden (" << name.fileOnly() << "): " << strerror(kernel_error) << std::endl;
        return 0;
    }
    return kernel_result;
}

void File::extend(filesize_t len)
{
    writeSomeAt(size(),std::string(len,0));
}

void File::save() const
{
    if (!pre_good())
        return;
    if (!is_open)
        throw Exception("Die Datei ist nicht offen","Database::File::save()");
    while (finish_kernel(::fsync(descriptor)));
        // nothing
    if (kernel_result == -1)
    {
        database->status = Database::Status::error;
        database->log << "Datei kann nicht auf speicher geschrieben werden (" << name.fileOnly() << "): " << strerror(kernel_error) << std::endl;
    }
}

std::string File::readSomeAt(filesize_t pos, filesize_t len) const
{
    if (!pre_good())
        return "";
    if (!is_open)
        throw Exception("Die Datei ist nicht offen.","Database::File::readSomeAt()");
    filesize_t s = size();
    if (pos>=s)
        return std::string();
    len = std::min(len,s-pos);

    filesize_t offset = 0;
    char* buffer = new char[len];
    std::string out;

    while (offset<len)
    {
        while(finish_kernel(::pread(
            descriptor,
            buffer + offset,
            len - offset,
            pos + offset
            )));
            //nothing
        
        if (kernel_result==-1)
        {
            delete[] buffer;
            database->status = Database::Status::error;
            database->log << "Fehler beim Lesen von " << name.fileOnly() << ": " << strerror(kernel_error) << std::endl;
            return "";
        }

        out.append(buffer + offset,kernel_result);
        offset += kernel_result;
    }

    delete[] buffer;
    return out;
}

void File::writeSomeAt(filesize_t pos, const std::string& data)
{
    if (!pre_good())
        return;
    if (!is_open)
        throw Exception("Die Datei ist nicht offen","Database::File::writeSomeAt()");

    filesize_t offset = 0;

    while (offset<data.size())
    {
        while(finish_kernel(::pwrite(
            descriptor,
            data.data() + offset,
            data.size() - offset,
            pos + offset
            )));
            //nothing

        if (kernel_result==-1)
        {
            database->status = Database::Status::error;
            database->log << name.fileOnly() << " konnte nicht beschrieben werden: " << strerror(kernel_error) << std::endl;
            return;
        }

        offset += kernel_result;
    }
}

template<class PrimT>
PrimT File::readAt(filesize_t where) const
{
    std::string result = readSomeAt(where,serial_size<PrimT>());
    return *reinterpret_cast<const PrimT*>(result.data());
}

template int          File::readAt(filesize_t) const;
template unsigned int File::readAt(filesize_t) const;
template double       File::readAt(filesize_t) const;
template char         File::readAt(filesize_t) const;
template bool         File::readAt(filesize_t) const;

template <class PrimT>
void File::writeAt(filesize_t where, PrimT what)
{
    writeSomeAt(
        where,
        std::string(reinterpret_cast<char*>(&what),serial_size<PrimT>())
    );
}

template void File::writeAt(filesize_t,int);
template void File::writeAt(filesize_t,unsigned int);
template void File::writeAt(filesize_t,double);
template void File::writeAt(filesize_t,char);
template void File::writeAt(filesize_t,bool);

bool File::finish_kernel(int result) const
{
    if (result == -1 && errno == EINTR) // temporary signal interrupt
        return true;
    kernel_result = result;
    kernel_error = errno;
    return false;
}

const mode_t File::FILEMODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;