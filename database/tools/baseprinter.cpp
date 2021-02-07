#define TC_DATABASE_INTERN
#include "../database.h"

using namespace tobilib;
using namespace database_detail;

#include <iostream>
#include <fstream>

void print_listfile(const ListFile& file)
{
    std::cout << "export listfile" << std::endl;
    std::fstream out (file.name.directory()+file.name.name+".csv",std::fstream::out);
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

    std::cout << "listfile exportiert" << std::endl;
    out.close();
}

void print_member(const Member& member, std::ostream& out)
{
    if (member.type.blockType == BlockType::t_char)
    {
        out << member.get<std::string>();
    }
    else if (member.type.amount>1)
    {
        for (const_MemberIterator it=member.cbegin();it!=member.cend();++it)
        {
            print_member(*it,out);
            out << " / ";
        }
    }
    else if (member.type.blockType == BlockType::t_int)
    {
        out << member.get<int>();
    }
    else if (member.type.blockType == BlockType::t_bool)
    {
        out << (member.get<bool>()?"1":"0");
    }
    else if (member.type.blockType == BlockType::t_double)
    {
        out << member.get<double>();
    }
    else if (member.type.blockType == BlockType::t_list)
    {
        out << member.fs->readAt<unsigned int>(member.position);
    }
    else if (member.type.blockType == BlockType::t_ptr)
    {
        out << member.fs->readAt<unsigned int>(member.position);
    }
}

void print_clusterfile(ClusterFile& file)
{
    std::fstream out (file.name.directory()+file.name.name+".csv", std::fstream::out);
    if (!out.good())
    {
        std::cout << file.name.fileOnly() << ": export fehlgeschlagen" << std::endl;
        return;
    }

    out << "first entry:;" << file.get_first_filled() << "\r\n";
    out << "last entry:;" << file.get_last_filled() << "\r\n";
    out << "first empty:;" << file.get_first_empty() << "\r\n";
    out << "last empty:;" << file.get_last_empty() << "\r\n";
    out << "capacity:;" << file.get_data_capacity() << "\r\n";
    out << "\r\n";
    out << "line index;is occupied;previous entry;next entry;referenced by;";
    for (MemberType& member: file.type.members)
        out << member.name << ";";
    out << "\r\n";

    for (LineIndex i=1;i<file.get_data_capacity();i++)
    {
        out << i << ";";
        out << (file.get_occupied(i)?"1":"0") << ";";
        out << file.get_previous(i) << ";";
        out << file.get_next(i) << ";";
        out << file.get_refcount(i) << ";";
        for (MemberType& member: file.type.members)
        {
            print_member(
                Member(member,file,i),
                out);
            out << ";";
        }
        out << "\r\n";
    }

    std::cout << file.name.fileOnly() << " wurde exportiert" << std::endl;
    out.close();
}

void printall(const std::string pfad)
{
    std::cout << "Exportiere " << pfad << std::endl;
    Database db (pfad);
    db.init();
    db.open();
    if (!db.is_good())
        return;
    for (ClusterFile& cf: db.clusters)
        print_clusterfile(cf);
    print_listfile(db.listfile);
}

int main(int argc, const char** args)
{
    if (argc<2)
    {
        std::cout << "Kein Pfad angegeben" << std::endl;
        return 0;
    }
    printall(args[1]);
    return 0;
}