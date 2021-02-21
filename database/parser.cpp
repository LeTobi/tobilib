#define TC_DATABASE_INTERN
#include "parser.h"
#include "database.h"

using namespace tobilib;
using namespace database_detail;

Parser::Parser(Database* db): Component(db)
{
    log.parent = &db->log;
    log.prefix = "Parser: ";
}

void Parser::parse_all()
{
    if (!pre_good())
        return;

    file.open((database->path+"struktur.txt").fullName(), file.in);
    if (!file.good())
    {
        database->status = Database::Status::error;
        log << "Datei konnte nicht geoeffnet werden." << std::endl;
        file.close();
        return;
    }
    parse_typenames();
    file.seekg(0);
    while (next_matches("type"))
        parse_cluster();
    if (!is_eof())
    {
        errorlog("es wird \"type\" erwartet.");
        file.close();
        return;
    }
    file.close();
}

const std::string Parser::valid_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-";

bool Parser::valid_name(const std::string& name)
{
    if (name=="int" || name=="char" || name=="double" || name=="bool" || name=="lists" || name=="status")
        return false;
    for (char c: name)
        if (valid_chars.find(c)==std::string::npos)
            return false;
    return true;
}

bool Parser::next_matches(const std::string& str)
{
    if (!pre_good())
        return false;

    std::streampos start = file.tellg();
    std::string next;
    file>>next;
    if (!file.good())
    {
        file.clear();
        file.seekg(start);
        return false;
    }
    file.seekg(start);
    return next==str;
}

bool Parser::is_eof()
{
    // hier müsste ich mal eine bessere lösung finden.
    while (file && file.peek()==' '||file.peek()=='\t'||file.peek()=='\r'||file.peek()=='\n')
        file.ignore();
    if (file.eof())
    {
        file.clear();
        return true;
    }
    if (!file)
    {
        errorlog("lesefehler");
        return true;
    }
    return false;
}

void Parser::parse_typenames()
{
    if (!pre_good())
        return;

    std::string command;
    while (file >> command)
    {
        if (command!="type")
            continue;
        file >> command;
        if (!file.good())
        {
            errorlog("Die Datei endet unerwartet. Es wird ein Clustername erwartet");
            return;
        }
        if (!valid_name(command))
        {
            errorlog("Unzulaessiger Clustername");
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
    if (!file.eof())
    {
        errorlog("lesefehler");
        return;
    }
    file.clear();
    return;
}

void Parser::parse_cluster()
{
    if (!pre_good())
        return;

    std::string name;
    // "type" überspringen
    if (! (file >> name >> name))
    {
        errorlog("Lesefehler");
        return;
    }
    ClusterFile* cluster = database->get_file(name);
    if (cluster==nullptr)
        throw Exception("Implementierungsfehler","Database::parse_cluster()");
    while (pre_good())
    {
        if (is_eof())
            break;
        if (next_matches("type"))
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
    if (!(file >> type))
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
        member.target_type = &(cf->type);
    }

    parse_arr_len(member.amount);

    if (!pre_good())
        return;

    member.size = member.amount * member.blockType.size;
    cluster.size += member.size;

    if (!(file >> member.name)) {
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
        if (!(file >> member.name)) {
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
    std::streampos start = file.tellg();
    StringPlus amount;
    if (! (file >> amount))
    {
        errorlog("lesefehler");
        out = 0;
        return;
    }
    if (!amount.isInt()) {
        file.seekg(start);
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
    log << "struktur.txt Position " << file.tellg() << ": " << msg << std::endl;
    database->status = Database::Status::error;
}