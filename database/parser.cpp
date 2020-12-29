#define TC_DATABASE_INTERN
#include "database.h"
#include "parser.h"

#include <iostream>

using namespace tobilib;
using namespace database_detail;

Parser::Parser(Database* db): Component(db), structurefile(db)
{
    structurefile.name = database->path + "struktur.txt";
}

void Parser::parse_all()
{
    structurefile.open();
    parse_typenames();
    structurefile.close();
    if (!pre_good())
        return;
    structurefile.open();
    while (next_matches("type"))
        parse_cluster();
    if (!pre_good())
        return;
    if (structurefile.fs.good()) {
        errorlog("Es wird \"type\" erwartet");
    }
}

const std::string Parser::valid_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-";

bool Parser::valid_name(const std::string& name)
{
    if (name == "int" || name == "char" || name == "double" || name == "bool")
        return false;
    for (char c: name)
        if (valid_chars.find(c)==std::string::npos)
            return false;
    return true;
}

bool Parser::next_matches(const std::string& str)
{
    std::streampos start = structurefile.fs.tellg();
    std::string next;
    structurefile.fs>>next;
    structurefile.fs.seekg(start);
    return next==str;
}

void Parser::parse_typenames()
{
    if (!pre_good())
        return;
    std::string command;
    while (structurefile.fs >> command)
    {
        if (command!="type")
            continue;
        structurefile.fs >> command;
        if (!structurefile.fs.good())
        {
            errorlog("Es wird ein Name erwartet");
            return;
        }
        if (!valid_name(command))
        {
            errorlog("Unzulaessiger name");
            return;
        }
        if (database->get_file(command)!=nullptr)
        {
            errorlog("doppelter Typenname");
            return;
        }
        database->clusters.emplace_back(database);
        database->clusters.back().type.name=command;
        database->clusters.back().name = database->path+(command+".data");
    }
}

void Parser::parse_cluster()
{
    if (!pre_good())
        return;
    std::string name;
    // "type" überspringen
    structurefile.fs >> name >> name;
    ClusterFile* cluster = database->get_file(name);
    if (cluster==nullptr)
        throw Exception("Implementierungsfehler","Database::parse_cluster()");
    while (pre_good())
    {
        if (next_matches("type"))
            break;
        if (!structurefile.fs.good())
            break;
        parse_block(cluster->type);
    }
}

void Parser::parse_block(ClusterType& cluster)
{
    if (!pre_good())
        return;
    std::string type;
    MemberType member;
    member.parent = &cluster;
    if (!(structurefile.fs >> type))
    {
        errorlog("Dateiende - Es wird ein BlockTyp erwartet");
        return;
    }

    if (type=="int") {
        member.blockType = BlockType::t_int;
    }
    else if (type=="char") {
        member.blockType = BlockType::t_char;
    }
    else if (type=="double") {
        member.blockType = BlockType::t_double;
    }
    else if (type=="bool") {
        member.blockType = BlockType::t_bool;
    }
    else {
        member.blockType = BlockType::t_ptr;
        ClusterFile* cf = database->get_file(type);
        if (cf==nullptr)
        {
            errorlog("Unbekannter Membertyp");
            return;
        }
        member.ptr_type = &(cf->type);
    }

    parse_arr_len(member.amount);

    if (!pre_good())
        return;

    member.size = member.amount * member.blockType.size;
    cluster.size += member.size;

    if (!(structurefile.fs >> member.name)) {
        errorlog("Name erwartet");
        return;
    }

    if (member.name=="[]") {
        if (member.blockType!=BlockType::t_ptr) {
            errorlog("Listen existieren nur fuer Referenzen");
            return;
        }
        if (member.amount!=1) {
            errorlog("Eine liste kann nicht teil eines arrays sein");
            return;
        }
        member.blockType = BlockType::t_list;
        if (!(structurefile.fs >> member.name)) {
            errorlog("Name erwartet");
            return;
        }
    }

    if (!valid_name(member.name)) {
        errorlog("ungueltiger Name");
        return;
    }

    if (cluster.contains(member.name)) {
        errorlog("doppelter Membername");
        return;
    }

    if (cluster.members.empty())
        member.parent_offset = 0l;
    else
        member.parent_offset = cluster.members.back().parent_offset + cluster.members.back().size;

    cluster.members.push_back(member);
}

void Parser::parse_arr_len(unsigned int& out)
{
    if (!pre_good())
        return;
    std::streampos start = structurefile.fs.tellg();
    StringPlus amount;
    structurefile.fs >> amount;
    if (!amount.isInt()) {
        structurefile.fs.seekg(start);
        out = 1;
        return;
    }
    out = amount.toInt();
    if (out<1) {
        errorlog("Die Anzahl muss groesser als 0 sein");
        return;
    }
}

void Parser::errorlog(const std::string& msg)
{
    database->log << "struktur.txt Position " << structurefile.fs.tellg() << ": " << msg << std::endl;
    database->status = Database::Status::error;
}