#include "commands.h"

using namespace tobilib;
using namespace database_tools;
using namespace detail;

std::string database_tools::command(Database& db, std::string cmd)
{
    if (!db.is_open())
        return "Die Datenbank ist nicht offen";
    
    std::stringstream input (cmd);

    std::string method;
    input >> method;

    if (method == "get")
    {
        Target target = resolve(db, input);
        return print(target);
    }
    else if (method == "set")
    {
        Target target = resolve(db,input);
        if (target.type == TargetType::invalid)
            return "Das Zielobjekt konnte nicht geparst werden.";
        if (!set(db,target,input))
            return "Zuweisung fehlgeschlagen";
        return "Zuweisung erfolgreich";
    }
    else if (method == "list")
    {
        Target target = resolve(db, input);
        if (target.type == TargetType::invalid)
            return "Das objekt konnte nicht geparst werden.";
        if (target.type != TargetType::list)
            return "Das objekt ist keine Memberliste.";
        std::string out;
        int count = 0;
        for (Database::Member item: target.member)
        {
            out += std::to_string(count++) + ": ";
            if (item->is_null())
                out+="nullptr\n";
            else
                out+= target.member.type().ptrType().name + "." + std::to_string(item->index()) + "\n";
        }
        if (count==0)
            return "empty list";
        return out;
    }
    else if (method == "emplace")
    {
        Target target = resolve(db, input);
        if (target.type == TargetType::clusterlist)
        {
            unsigned int index = target.list.emplace().index();
            return std::string("Die Liste wurde erweitert.\nNeues Element bei: ") + std::to_string(index);
        }
        else if (target.type == TargetType::list)
        {
            if (target.member.is_null())
                return "Das Objekt ist ungueltig.";
            target.member.emplace();
            return "Die Liste wurde erweitert.\nNeues Element: 0";
        }
        else
        {
            return "Das objekt kann nicht erweitert werden.";
        }
    }
    else if (method == "erase")
    {
        Target target = resolve(db, input);
        if (target.type == TargetType::cluster)
        {
            if (target.cluster.is_null())
                return "Das Objekt ist ungueltig";
            if (target.cluster.reference_count() != 0)
                return "Das Objekt ist in verwendung und kann nicht geloescht werden.";
            target.cluster.erase();
            return "Das Element wurde geloescht.";
        }
        else if (target.type == TargetType::list)
        {
            if (target.member.is_null())
                return "Das Objekt ist ungueltig";
            unsigned int i;
            input >> i;
            if (!input)
                return "Es muss ein Index angegeben werden.";
            Database::MemberIterator it = target.member.begin();
            while (i-- > 0)
                ++it;
            if (it == target.member.end())
                return "Der Index ist ungueltig.";
            target.member.erase(it);
            return "Das Element wurde geloescht.";
        }
        else
        {
            return "Das objekt kann nicht geloescht werden.";
        }
    }
    else
    {
        return "unbekannter Befehl";
    }
}

Target detail::resolve(Database& db, std::istream& input)
{
    StringPlus tstr;
    input >> tstr;
    std::vector<StringPlus> names = tstr.split(".");
    if (names.size()<1)
        return Target();
    Database::ClusterType ctype = db.getType(names.front());
    if (ctype == Database::ClusterType::invalid)
        return Target();
    Target source;
    source.type = TargetType::clusterlist;
    source.list = db.list(ctype.name);
    names.erase(names.begin());
    return resolve(source,names);
}

Target detail::resolve(Target source, std::vector<StringPlus>& names)
{
    if (names.empty())
        return source;
    
    if (source.type == TargetType::clusterlist)
    {
        if (!names.front().isInt())
            return Target();
        int index = names.front().toInt();
        Target target;
        target.type = TargetType::cluster;
        target.cluster = source.list[index];
        names.erase(names.begin());
        return resolve(target,names);
    }
    if (source.type == TargetType::cluster)
    {
        if (!source.cluster.type().contains(names.front()))
            return Target();
        Database::Member next = source.cluster[names.front()];
        names.erase(names.begin());
        return resolve(select_type(next), names);
    }
    if (source.type == TargetType::primitiveMember)
    {
        return Target();
    }
    if (source.type == TargetType::array)
    {
        if (!names.front().isInt())
            return Target();
        int index = names.front().toInt();
        if (index<0 || index >= source.member.type().amount)
            return Target();
        names.erase(names.begin());
        return resolve(select_type(source.member[index]), names);
    }
    if (source.type == TargetType::pointer)
    {
        Target next;
        next.type = TargetType::cluster;
        next.cluster = *source.member;
        return resolve(next,names);
    }
    if (source.type == TargetType::list)
    {
        if (!names.front().isInt())
            return Target();
        int index = names.front().toInt();
        if (index<0)
            return Target();
        Database::MemberIterator it = source.member.begin();
        while (index-- > 0)
            ++it;
        names.erase(names.begin());
        return resolve(select_type(*it), names);
    }
    if (source.type == TargetType::invalid)
    {
        return source;
    }

    throw Exception("Unhandled Type","tobilib::database_tools::detail::resolve()");
}

Target detail::select_type(Database::Member member)
{
    Target out;
    out.member = member;

    if (member.is_null())
    {
        out.type = TargetType::invalid;
        return out;
    }
    if (member.type().amount > 1)
    {
        out.type = TargetType::array;
        return out;
    }
    if (member.type().blockType == Database::BlockType::t_ptr)
    {
        out.type = TargetType::pointer;
        return out;
    }
    if (member.type().blockType == Database::BlockType::t_list)
    {
        out.type = TargetType::list;
        return out;
    }
    out.type = TargetType::primitiveMember;
    return out;
}

bool detail::set(Database& db, Target target, std::istream& input)
{
    if (target.type == TargetType::pointer)
    {
        Target item = resolve(db,input);
        if (item.type != TargetType::cluster)
            return false;
        if (item.cluster.type() != target.member.type().ptrType())
            return false;
        target.member.set(item.cluster);
        return true;
    }
    if (target.type == TargetType::primitiveMember)
    {
        if (target.member.type().blockType == Database::BlockType::t_bool)
        {
            bool val;
            input >> val;
            if (!input)
                return false;
            target.member.set(val);
            return true;
        }
        if (target.member.type().blockType == Database::BlockType::t_char)
        {
            char val;
            input >> val;
            if (!input)
                return false;
            target.member.set(val);
            return true;
        }
        if (target.member.type().blockType == Database::BlockType::t_int)
        {
            int val;
            input >> val;
            if (!input)
                return false;
            target.member.set(val);
            return true;
        }
        if (target.member.type().blockType == Database::BlockType::t_double)
        {
            double val;
            input >> val;
            if (!input)
                return false;
            target.member.set(val);
            return true;
        }
        return false;
    }
    if (target.type == TargetType::array)
    {
        if (target.member.type().blockType != Database::BlockType::t_char)
            return false;
        
        std::string val;
        char c;
        input >> c;
        if (c != '"')
            return false;
        while (input)
        {
            c = input.get();
            if (c=='"')
                break;
            if (c=='\\')
            {
                c = input.get();
                if (c=='"') { }
                else if (c=='n') { c = '\n';}
                else if (c=='\\') { c = '\\';}
                else if (c=='r') { c = '\r';}
                else if (c=='t') { c = '\t';}
                else return false;
            }
            val += c;
        }
        target.member.set( val );
        return true;
    }
    return false;
}

std::string detail::print(Target target)
{
    if (target.type == TargetType::invalid)
    {
        return "Invalid name";
    }
    if (target.type == TargetType::clusterlist)
    {
        return std::string("List of: ")+ target.list.type().name;
    }
    if (target.type == TargetType::array)
    {
        if (target.member.type().blockType == Database::BlockType::t_char)
            return target.member.get<std::string>();
        return "array";
    }
    if (target.type == TargetType::cluster)
    {
        if (target.cluster.is_null())
            return "null";
        return target.cluster.type().name + "." + std::to_string(target.cluster.index());
    }
    if (target.type == TargetType::list)
    {
        return std::string("list of ") + target.member.type().ptrType().name;
    }
    if (target.type == TargetType::pointer)
    {
        Database::Cluster ptr = *target.member;
        if (ptr.is_null())
            return "nullptr";
        return std::string("pointer to ") + ptr.type().name + "." + std::to_string(ptr.index());
    }
    if (target.type == TargetType::primitiveMember)
    {
        if (target.member.type().blockType == Database::BlockType::t_bool)
        {
            return std::to_string(target.member.get<bool>());
        }
        if (target.member.type().blockType == Database::BlockType::t_char)
        {
            return std::to_string(target.member.get<char>());
        }
        if (target.member.type().blockType == Database::BlockType::t_int)
        {
            return std::to_string(target.member.get<int>());
        }
        if (target.member.type().blockType == Database::BlockType::t_double)
        {
            return std::to_string(target.member.get<double>());
        }
    }
    throw Exception("Unhandled Target Type","database_tools::detail::print()");
}