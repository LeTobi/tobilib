#include "typeparser.h"
#include "../stringplus/stringplus.h"

using namespace tobilib;
using namespace database;

const std::string TypeParser::valid_chars = "0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

bool TypeParser::valid_name(const std::string& name)
{
    if (name=="struktur")
        return false;
    if (name=="lists")
        return false;
    for (auto& c: name)
        if (valid_chars.find(c)==std::string::npos)
            return false;
    return true;
}

bool TypeParser::parse(DatabaseInfo& info)
{
// clear info
    info.types.clear();
    info.path = path;

// Open File
    fs.open((path+"struktur.txt").fullName().toString(),fs.in);
    if (!fs.good())
    {
        result = Result::file_access_error;
        log << "Es kann nicht auf die Datei zugegriffen werden" << std::endl;
        fs.close();
        return false;
    }
    
// parse
    parse_all_typenames(info);
    std::string command;
    while (fs>>command)
    {
        if (command!="type")
        {
            result = Result::format_error;
            log << "Unbekannte Anweisung: " << command << std::endl;
            fs.close();
            return false;
        }
        if (!(fs>>command))
        {
            result = Result::file_access_error;
            log << "Der Dateizugriff ist abgebrochen, wtf?" << std::endl;
            fs.close();
            return false;
        }
        ClusterInfo* cluster = info.find(command);
        if (cluster==nullptr)
            throw ("Ein Typ wurde nicht wiedergefunden","database::TypeParser");
        if (!parse_cluster(info,*cluster))
        {
            fs.close();
            return false;
        }
    }

// success
    result = Result::success;
    fs.close();
    return true;
}

bool TypeParser::write(const DatabaseInfo& info)
{
    fs.open((path+"struktur.txt").fullName(),fs.out);
    if (!fs.good())
    {
        log << "Die Strukturdatei konnte nicht beschrieben werden." << std::endl;
        result = Result::file_access_error;
        fs.close();
        return false;
    }
    
    for (auto& cluster: info.types)
    {
        fs << "type " << cluster.name << std::endl;
        for (auto& block: cluster.segmentation)
        {
            fs << "\t";

            switch (block.type) {
            case BlockType::t_int:
                fs << "int ";
                write_primitive(block);
                break;
            case BlockType::t_char:
                fs << "char ";
                write_primitive(block);
                break;
            case BlockType::t_decimal:
                fs << "decimal ";
                write_primitive(block);
                break;
            case BlockType::t_bool:
                fs << "bool ";
                write_primitive(block);
                break;
            case BlockType::t_cluster_ptr:
                fs << block.ptr_type->name << " " << block.name;
                break;
            case BlockType::t_list_ptr:
                fs << block.ptr_type->name << " [] " << block.name;
                break;
            }

            fs << std::endl;
        }
    }

    if (!fs.good())
    {
        log << "Fehler bei beschreiben der Datei" << std::endl;
        fs.close();
        return false;
    }
    fs.close();
    return true;
}

bool TypeParser::parse_all_typenames(DatabaseInfo& info)
{
    std::string command;
    while (fs>>command)
    {
        if (command=="type")
        {
            if (!(fs>>command))
            {
                result = Result::format_error;
                log << "Position " << fs.tellg() << " - Es wird eine Namensgebung erwartet" << std::endl;
                return false;
            }
            if (!valid_name(command))
            {
                result = Result::format_error;
                log << "Position " << fs.tellg() << " - Ungueltiger name: " << command << std::endl;
                return false;
            }
            info.types.emplace_back();
            info.types.back().name = command;
        }
    }
    fs.clear();
    fs.seekg(0);
    return true;
}

bool TypeParser::parse_cluster(DatabaseInfo& dbinfo, ClusterInfo& info) {
    std::string command;
    std::streampos pos = fs.tellg();
    while (fs>>command)
    {
        fs.seekg(pos);
        if (command=="type")
            return true;
        info.segmentation.emplace_back();
        if (!parse_block(dbinfo,info.segmentation.back()))
            return false;
        pos = fs.tellg();
    }
    return true;
}

bool TypeParser::parse_block(DatabaseInfo& dbinfo, BlockInfo& info) {
    std::string type;
    if (!(fs>>type))
    {
        result = Result::format_error;
        log << "Dateiende - Es wird ein BlockTyp erwartet" << std::endl;
        return false;
    }
    if (type=="int") {
        info.type=BlockType::t_int;
    }
    else if (type=="char") {
        info.type=BlockType::t_char;
    }
    else if (type=="decimal") {
        info.type=BlockType::t_decimal;
    }
    else if (type=="bool"){
        info.type=BlockType::t_bool;
    }
    else {
        info.type=BlockType::t_cluster_ptr;
        info.ptr_type = dbinfo.find(type);
        if (info.ptr_type==nullptr)
        {
            result = Result::format_error;
            log << "Position " << fs.tellg() << " - Der Blocktyp kann nicht identifizert werden" << std::endl;
            return false;
        }
    }

    if (!parse_arr_len(info.amount))
        info.amount=1;
    
    if (info.amount==0) {
        if (info.type != BlockType::t_cluster_ptr)
        {
            log << "Position " << fs.tellg() << " - Eine liste kann nicht aus primitiven Typen bestehen" << std::endl;
            result = Result::format_error;
            return false;
        }
        info.amount=1;
        info.type=BlockType::t_list_ptr;
    }

    if (!(fs>>info.name))
    {
        result = Result::format_error;
        log << "Dateiende - Es wird ein Blockname erwartet" << std::endl;
        return false;
    }

    if (!valid_name(info.name))
    {
        result = Result::format_error;
        log << "Position " << fs.tellg() << " - Ungueltiger Name" << std::endl;
        return false;
    }

    return true;
}

bool TypeParser::parse_arr_len(unsigned int& out) {
    std::streampos start = fs.tellg();
    StringPlus amount;
    if (!(fs>>amount))
    {
        out = 1;
        return true;
    }
    if (amount=="[]") {
        out = 0;
        return true;
    }
    if (!amount.isInt()) {
        fs.seekg(start);
        out = 1;
        return true;
    }
    out = amount.toInt();
    if (out<1) {
        result = Result::format_error;
        log << "Position " << fs.tellg() << " - Die laenge muss >= 1 sein" << std::endl;
        return false;
    }
    return true;
}

void TypeParser::write_primitive(const BlockInfo& block)
{
    if (block.amount>1)
        fs << block.amount << " ";
    fs << block.name;
}