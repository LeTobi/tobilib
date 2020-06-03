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
    while (check("type"))
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

bool Parser::check(const std::string& str)
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
        if (database->get_cluster(command)!=nullptr)
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
    ClusterFile* cluster = database->get_cluster(name);
    if (cluster==nullptr)
        throw Exception("Implementierungsfehler","Database::parse_cluster()");
    while (pre_good())
    {
        if (check("type"))
            return;
        if (!structurefile.fs.good())
            return;
        parse_block(cluster->type);
    }
}

void Parser::parse_block(ClusterType& cluster)
{
    if (!pre_good())
        return;
    std::string type;
    MemberType member;
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
        ClusterFile* cf = database->get_cluster(type);
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

    std::string name;
    if (!(structurefile.fs >> name)) {
        errorlog("Name erwartet");
        return;
    }

    if (name=="[]") {
        if (member.blockType!=BlockType::t_ptr) {
            errorlog("Listen existieren nur fuer Referenzen");
            return;
        }
        if (member.amount!=1) {
            errorlog("Eine liste kann nur einfach vorhanden sein");
            return;
        }
        member.blockType = BlockType::t_list;
        if (!(structurefile.fs >> name)) {
            errorlog("Name erwartet");
            return;
        }
    }

    if (!valid_name(name)) {
        errorlog("ungueltiger Name");
        return;
    }

    if (cluster.members.count(name)>0) {
        errorlog("doppelter Membername");
        return;
    }

    cluster.members[name] = member;
    return;
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