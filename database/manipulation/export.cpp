#define TC_DATABASE_INTERN
#include "export.h"

using namespace tobilib;
using namespace database_tools;
using namespace database_detail;
using namespace detail;

Result database_tools::export_database(const Database& db)
{
    return export_database(db,db.path);
}

Result database_tools::export_database(const Database& db, const FileName& path)
{
    if (!db.is_good())
        return "Die Datenbank ist im Fehlerzustand";
    if (!db.is_open())
        return "Die Datenbank ist nicht offen";
    Result res;
    for (const ClusterFile& cf: db.clusters)
    {
        res = print_clusterfile(cf,path);
        if (!res)
            return res;
    }
    return print_listfile(db.listfile,path);
}

Result database_tools::export_table(const Database& db, const std::string& tname)
{
    return export_table(db,tname,db.path);
}

Result database_tools::export_table(const Database& db, const std::string& tname, const FileName& path)
{
    if (tname=="Lists")
        return print_listfile(db.listfile, path);
    const ClusterFile* cf = db.get_file(tname);
    if (cf==nullptr)
        return "Die Tabelle existiert nicht";
    return print_clusterfile(*cf,path);
}

Result detail::print_listfile(const ListFile& file, const FileName& path)
{
    std::fstream out (path.directory()+file.name.name+".csv",std::fstream::out);
    out << "first empty:;" << file.get_first_empty() << "\r\n";
    out << "last empty:;" << file.get_last_empty() << "\r\n";
    out << "capacity:;" << file.get_data_capacity() << "\r\n";
    out << "\r\n";
    out << "line index;previous;next;target\r\n";

    for (LineIndex i=1;i<file.get_data_capacity();i++)
    {
        out << i << ";";
        out << file.get_previous(i) << ";";
        out << file.get_next(i) << ";";
        out << file.readAt<unsigned int>(file.data_location(i)) << ";\r\n";
    }

    if (!out.good())
    {
        return "Listfile konnte nicht exportiert werden.";
    }

    out.close();
    return Result();
}

void detail::print_member(const Member& member, std::ostream& out)
{
    if (member.type().blockType == BlockType::t_char)
    {
        out << member.get<std::string>();
    }
    else if (member.type().amount>1)
    {
        for (const_MemberIterator it=member.cbegin();it!=member.cend();++it)
        {
            print_member(*it,out);
            out << " / ";
        }
    }
    else if (member.type().blockType == BlockType::t_int)
    {
        out << member.get<int>();
    }
    else if (member.type().blockType == BlockType::t_bool)
    {
        out << (member.get<bool>()?"1":"0");
    }
    else if (member.type().blockType == BlockType::t_double)
    {
        out << member.get<double>();
    }
    else if (member.type().blockType == BlockType::t_list)
    {
        out << member.fs->readAt<unsigned int>(member.position);
    }
    else if (member.type().blockType == BlockType::t_ptr)
    {
        out << member.fs->readAt<unsigned int>(member.position);
    }
    else
    {
        throw Exception("Implementierungsfehler","database_tools::print_member()");
    }
}

Result detail::print_clusterfile(const ClusterFile& file, const FileName& path)
{
    FileName exportpath = path.directory() + file.name.name + ".csv";
    std::fstream out (exportpath.fullName(), std::fstream::out);
    if (!out.good())
        return (exportpath.fullName() + ": export fehlgeschlagen").toString().c_str();

    out << "first entry:;" << file.get_first_filled() << "\r\n";
    out << "last entry:;" << file.get_last_filled() << "\r\n";
    out << "first empty:;" << file.get_first_empty() << "\r\n";
    out << "last empty:;" << file.get_last_empty() << "\r\n";
    out << "capacity:;" << file.get_data_capacity() << "\r\n";
    out << "\r\n";
    out << "line index;is occupied;previous entry;next entry;referenced by;";
    for (const MemberType& member: file.type.members)
        out << member.name << ";";
    out << "\r\n";

    for (LineIndex i=1;i<file.get_data_capacity();i++)
    {
        out << i << ";";
        out << (file.get_occupied(i)?"1":"0") << ";";
        out << file.get_previous(i) << ";";
        out << file.get_next(i) << ";";
        out << file.get_refcount(i) << ";";
        for (const MemberType& member: file.type.members)
        {
            print_member(
                Member(member,file,i),
                out);
            out << ";";
        }
        out << "\r\n";
    }
    out.close();
    return Result();
}