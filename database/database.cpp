#include "database.h"
#include "parser.h"

using namespace tobilib;

Database::Database(): listfile(this), status(Status::empty)
{ }

Database::Database(const FileName& _path): listfile(this), status(Status::empty), path(_path)
{ }

void Database::setPath(const FileName& fname)
{
    path = fname;
}

bool Database::init()
{
    if (status!=Status::empty)
        throw Exception("Die Datenbank ist bereits geladen","Database::init()");
    listfile.name = path+"lists.data";
    database_detail::Parser parser (this);
    parser.parse_all();
    if (!is_good())
        return false;
    status = Status::closed;
    return true;
}

bool Database::is_init() const
{
    return status==Status::closed || status==Status::open;
}

bool Database::open()
{
    if (status==Status::error)
        return false;
    if (status!=Status::closed) {
        status=Status::error;
        log << "open() mit falschem status" << std::endl;
        return false;
    }
    listfile.open();
    for (auto& cf: clusters) {
        cf.open();
    }
    status = Status::open;
    return is_good();
}

bool Database::is_open() const
{
    return status==Status::open;
}

void Database::close()
{
    if (status==Status::error)
        return;
    if (status != Status::open) {
        status = Status::error;
        log << "close() mit falschem status" << std::endl;
        return;
    }
    listfile.close();
    for (auto& cf: clusters)
        cf.close();
    status = Status::closed;
}

void Database::clear()
{
    clusters.clear();
    close();
    status = Status::empty;
}

bool Database::is_good() const
{
    return status!=Status::error;
}

Database::ClusterList Database::list(const std::string& name)
{
    if (!is_good())
        return ClusterList();
    ClusterFile* cluster = get_cluster(name);
    if (cluster==nullptr)
        throw Exception("Ungueltiger Typenname","Database::list()");
    return ClusterList(this,cluster);
}

const Database::ClusterList Database::list(const std::string& name) const
{
    return const_cast<Database*>(this)->list(name);
}

Database::ClusterFile* Database::get_cluster(const std::string& name)
{
    for (auto& cf: clusters) {
        if (cf.type.name==name)
            return &cf;
    }
    return nullptr;
}

Database::ClusterFile* Database::get_cluster(const ClusterType* cltype)
{
    for (auto& cf: clusters) {
        if (&cf.type == cltype)
            return &cf;
    }
    throw Exception("Clustertype ohne Clusterfile","Database::get_cluster(ClusterType*)");
}