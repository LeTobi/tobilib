#define TC_DATABASE_INTERN
#include "database.h"
#include "parser.h"

using namespace tobilib;

Database::Database(): listfile(this), statusfile(this), status(Status::empty)
{ }

Database::Database(const FileName& _path): listfile(this), statusfile(this), status(Status::empty), path(_path)
{ }

void Database::setPath(const FileName& fname)
{
    path = fname;
}

bool Database::init()
{
    if (status==Status::error)
        return false;
    if (status!=Status::empty)
        throw Exception("Die Datenbank ist bereits geladen","Database::init()");
    listfile.name = path+"lists.data";
    statusfile.name = path+"status.data";
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
    statusfile.open();
    for (auto& cf: clusters)
        cf.open();

    if (statusfile.get_fallback_enabled())
    {
        listfile.restore();
        for (auto& cf: clusters)
            cf.restore();
        statusfile.set_fallback_enabled(false);
    }
    else
    {
        listfile.confirm();
        for (auto& cf: clusters)
            cf.confirm();
    }
    
    if (status != Status::error)
    {
        status = Status::open;
        return true;
    }
    else
    {
        return false;
    }
}

bool Database::is_open() const
{
    return status==Status::open;
}

void Database::close()
{
    if (critical_operation_running())
        throw Exception("Es sind empfindliche Zugriffe ausstehend.","Database::close()");
    listfile.close();
    statusfile.close();
    for (auto& cf: clusters)
        cf.close();
    if (status != Status::error)
        status = Status::closed;
}

void Database::clear()
{
    if (critical_operation_running())
        throw Exception("Es sind empfindliche Zugriffe ausstehend.","Database::clear()");
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
    ClusterFile* cluster = get_file(name);
    if (cluster==nullptr)
        throw Exception("Ungueltiger Typenname","Database::list()");
    return ClusterList(this,cluster);
}

const Database::ClusterList Database::list(const std::string& name) const
{
    return const_cast<Database*>(this)->list(name);
}

const Database::ClusterType& Database::getType(const std::string& name) const
{
    if (!is_good())
        return ClusterType::invalid;
    const ClusterFile* cf = get_file(name);
    if (cf==nullptr)
        return ClusterType::invalid;
    return cf->type;
}

std::vector<std::string> Database::getTypes() const
{
    std::vector<std::string> out;
    if (!is_good())
        return out;
    for (const auto& cf: clusters)
        out.push_back(cf.type.name);
    return out;
}

FlagRequest Database::begin_critical_operation()
{
    if (!critical_operation.is_requested())
    {
        statusfile.set_fallback_enabled(true);
    }
    return critical_operation.request();
}

bool Database::critical_operation_running() const
{
    return critical_operation.is_requested();
}

void Database::end_critical_operation(FlagRequest id)
{
    critical_operation.dismiss(id);
    critical_operation.events.clear();
    if (!critical_operation.is_requested())
    {
        statusfile.set_fallback_enabled(false);
        listfile.confirm();
        for (ClusterFile& cf: clusters)
            cf.confirm();
    }
}

Database::ClusterFile* Database::get_file(const std::string& name)
{
    for (auto& cf: clusters) {
        if (cf.type.name==name)
            return &cf;
    }
    return nullptr;
}

const Database::ClusterFile* Database::get_file(const std::string& name) const
{
    return const_cast<Database*>(this)->get_file(name);
}

Database::ClusterFile* Database::get_file(const ClusterType* cltype)
{
    for (auto& cf: clusters) {
        if (&cf.type == cltype)
            return &cf;
    }
    throw Exception("Clustertype ohne Clusterfile","Database::get_file(ClusterType*)");
}

const Database::ClusterFile* Database::get_file(const ClusterType* cltype) const
{
    return const_cast<Database*>(this)->get_file(cltype);
}