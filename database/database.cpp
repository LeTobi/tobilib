#include "database.h"
#include "../general/exception.hpp"

using namespace tobilib;
using namespace database;

Element Access::list(const std::string& name)
{
    if (!is_init())
        throw Exception("Der Zugriff wurde nicht initialisiert","database::Access");
    ClusterInfo* cluster = typeinfo.find(name);
    if (cluster==nullptr)
        throw Exception("Der Name ist nicht teil der Datenbank","database::Access");
    Element out (Element::Type::cluster_file,this);
    out.cluster_file_info = cluster;
    out.fs = &files[cluster].stream;
    out.position=0;
    return out;
}

bool Access::init(const FileName& fname)
{
    if (state != State::empty)
        clear();
    TypeParser parser;
    parser.log = log;
    parser.path = fname;
    DatabaseInfo info;
    if (!parser.parse(info))
    {
        state = State::error;
        return false;
    }
    return init(info);
}

bool Access::init(const DatabaseInfo& dbinfo)
{
    if (state!= State::empty)
        clear();
    typeinfo = dbinfo;
    listfile.name = dbinfo.path+"lists.txt";
    for (auto& cluster: typeinfo.types) {
        files[&cluster].name = dbinfo.path + (cluster.name+".txt");
    }
    state = State::closed;
    return true;
}

bool Access::is_init() const
{
    return state != State::empty;
}

bool Access::open()
{
    if (state==State::open)
        return true;
    if (state!=State::closed)
        throw Exception("Der Zugriff kann in diesem Zustand nicht geoeffnet werden.","database::Access");

    if (!openFile(listfile)) {
        state = State::error;
        return false;
    }
    if (listfile.size()%listItemSize != 0)
    {
        state=State::error;
        log << "Die Listen-Datei hat eine ungueltige Laenge" << std::endl;
        return false;
    }
    for (auto& item: files)
    {
        if (!openFile(item.second)) {
            state=State::error;
            return false;
        }
        if (item.second.size()%clusterSize(*item.first) != 0)
        {
            state=State::error;
            log << "Die Datei hat eine ungueltige laenge: " << item.second.name << std::endl;
            return false;
        }
    }
    
    state = State::open;
    return true;
}

bool Access::is_good() const
{
    return state!=State::error;
}

bool Access::is_open() const
{
    return state==State::open;
}

void Access::close()
{
    if (state==State::closed)
        return;
    if (state!=State::open)
        throw Exception("Der Zugriff kann in diesem Zustand nicht geschlossen werden","database::Access");
    listfile.stream.close();
    for (auto& item: files) {
        item.second.stream.close();
    }
    state=State::closed;
}

void Access::clear()
{
    files.clear();
    listfile.stream.close();
    state = State::empty;
}

unsigned int Access::File::size() const
{
    stream.seekg(0);
    std::streampos start = stream.tellg();
    stream.seekg(0,stream.end);
    std::streampos end = stream.tellg();
    stream.seekg(0);
    if (!stream.good())
        return 0;
    return end-start;
}

bool Access::openFile(File& file)
{
    file.stream.close();
    file.stream.open(file.name.fullName(),file.stream.in|file.stream.out);
    if (file.stream.good())
        return true;
    file.stream.close();
    file.stream.open(file.name.fullName(),file.stream.out);
    if (!file.stream.good()) {
        file.stream.close();
        log << "Kein Zugriff auf: " << file.name << std::endl;
        return false;
    }
    file.stream.close();
    log << "Eine Datei wurde neu angelegt: " << file.name << std::endl;
    file.stream.open(file.name.fullName(),file.stream.in|file.stream.out);
    if (file.stream.good())
        return true;
    file.stream.close();
    log << "Lesezugriff fehlgeschlagen: " << file.name << std::endl;
    return false;
}

// listItemSize: occupation, cluster_ptr, list_item_ptr, endl
const unsigned int Access::listItemSize = 1 + 3 + 3 + 2;

std::streampos Access::clusterSize(const ClusterInfo& cluster)
{
    std::streampos sum=0;
    for (auto& block: cluster.segmentation)
        sum+=blockSize(block);
    sum+=1; // occupied or not
    sum+=2; // endl
    return sum;
}

std::streampos Access::blockSize(const BlockInfo& block)
{
    return block.amount * primitiveSize(block);
}

std::streampos Access::primitiveSize(const BlockInfo& block)
{
    switch (block.type)
    {
    case BlockType::t_int:
        return sizeof(int);
    case BlockType::t_char:
        return 1;
    case BlockType::t_decimal:
        return sizeof(double);
    case BlockType::t_bool:
        return 1;
    case BlockType::t_cluster_ptr:
        return 3;
    case BlockType::t_list_ptr:
        return 3;
    }
    return 0;
}

unsigned int Access::read_index(std::fstream& fs) const
{
    unsigned int out;
    for (unsigned int i=0;i<3;i++) {
        unsigned int z = fs.get();
        out+= z<<((2-i)*8);
    }
    return out;
}

void Access::write_index(std::fstream& fs, unsigned int index)
{
    if (index>=(1u<<(8*3)))
        throw Exception("Der Index ist zu gross um geschrieben zu werden");
    for (unsigned int i=0;i<3;i++) {
        fs.put((index>>((2-i)*8))%256);
    }
}

void Access::initialize(const ClusterInfo& info, unsigned int index)
{
    std::streampos size = clusterSize(info);
    std::streampos pos = size*index;
    std::fstream& fs = files[const_cast<ClusterInfo*>(&info)].stream;
    fs.seekp(pos);
    fs << '*' << std::string(size-(std::streampos)3,0) << "\r\n";
    if (!fs.good())
    {
        log << "Zugriffsfehler" << std::endl;
        state = State::error;
    }
}