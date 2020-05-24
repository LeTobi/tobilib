#include "database.h"
#include "../general/exception.hpp"
#include "../stringplus/stringplus.h"

#warning debug mode
#include <iostream>

template<class PrimT>
inline std::streampos serial_size()
{
    return sizeof(PrimT);
}

template<bool>
inline std::streampos serial_size()
{
    return sizeof(char);
}

template<class PrimT>
bool serial_print(std::fstream& fs, PrimT out)
{
    char* data = reinterpret_cast<char*>(&out);
    for (int i=0;i<sizeof(PrimT);i++) {
        fs.put(data[i]);
    }
    return fs.good();
}

template<bool>
bool serial_print(std::fstream& fs, bool out)
{
    fs << out?'1':'0';
    return fs.good();
}

template<class PrimT>
bool serial_read(std::fstream& fs, PrimT& in) {
    char* data = reinterpret_cast<char*>(&in);
    for (int i=0;i<sizeof(PrimT);i++) {
        data[i] = fs.get();
    }
    return fs.good();
}

template<bool>
bool serial_read(std::fstream& fs, bool& in) {
    in = fs.get()=='1';
    return fs.good();
}

using namespace tobilib;

const Database::BlockType Database::BlockType::t_int   (1, serial_size<int>());
const Database::BlockType Database::BlockType::t_char  (2, serial_size<char>());
const Database::BlockType Database::BlockType::t_double(3, serial_size<double>());
const Database::BlockType Database::BlockType::t_bool  (4, serial_size<bool>());
const Database::BlockType Database::BlockType::t_list  (5, serial_size<unsigned int>());
const Database::BlockType Database::BlockType::t_ptr   (6, serial_size<unsigned int>());

std::streampos Database::MemberType::size() const
{
    return amount*blockType.size;
}

bool Database::ClusterType::operator== (const ClusterType& other) const
{
    return name==other.name;
}

bool Database::ClusterType::operator!= (const ClusterType& other) const
{
    return !(*this == other);
}

// Cluster:
// | first    | last     | first-empty | last-empty  | ...  | "\r\n" |
// | occupied | previous | next        | ref-count   | data | "\r\n" |
std::streampos Database::ClusterType::size() const
{
    std::streampos out = serial_size<bool>() + 3*serial_size<unsigned int>()+ 2*serial_size<char>();
    std::streampos min_out = 4*serial_size<unsigned int>() + 2*serial_size<char>();
    for (auto& mem: members) {
        out+=mem.second.size();
    }
    return std::max(min_out,out);
}

std::streampos Database::ClusterType::offsetOf(const std::string& name) const
{
    if (members.count(name)==0)
        throw Exception("Implementierungsfehler","Database::ClusterType::offsetOf");
    std::streampos out = serial_size<bool>() + 3*serial_size<unsigned int>();
    for (auto& mem: members) {
        if (mem.first==name)
            return out;
        out+=mem.second.size();
    }
    // never reached!
    return 0;
}

Database::Element::Element(): type(Type::null)
{ }

void Database::Element::copy (const Element& other)
{
    type = other.type;
    fs = other.fs;
    position = other.position;
    database = other.database;
    memberType = other.memberType;
}

bool Database::Element::is_null() const
{
    if (type==Type::null)
        return true;
    if (!database->is_good())
        return true;
    if (type==Type::cluster && !clusterFile()->get_occupied(position))
        return true;
   return false;
}

bool Database::Element::operator==(const Element& other) const
{
    if (type==Type::null && other.type==Type::null)
        return true;
    return
        type == other.type &&
        fs == other.fs &&
        position == other.position &&
        database == other.database;
}

bool Database::Element::operator!=(const Element& other) const
{
    return !(*this==other);
}

Database::Element::operator int() const
{
    if (is_null())
        return 10;
    if (type!=Type::block || memberType.blockType!=BlockType::t_int)
        throw ("Type-Error","Database::Element::operator int");
    return fs->readAt<int>(position);
}

Database::Element::operator char() const
{
    if (is_null())
        return 0;
    if (type!=Type::block || memberType.blockType!=BlockType::t_char)
        throw ("Type-Error","Database::Element::operator char");
    return fs->readAt<char>(position);
}

Database::Element::operator std::string() const
{
    if (is_null())
        return "";
    if (type!=Type::array || memberType.blockType!=BlockType::t_char)
        throw ("Type-Error","Database::Element::operator std::string");
    std::string out;
    for (std::streampos i=0;i<memberType.amount;i+=1)
    {
        char c = fs->readAt<char>(position+i);
        if (c==0)
            break;
        out+=c;
    }
    return out;
}

Database::Element::operator double() const
{
    if (is_null())
        return 0;
    if (type!=Type::block || memberType.blockType!=BlockType::t_double)
        throw ("Type-Error","Database::Element::operator double");
    return fs->readAt<double>(position);
}

Database::Element::operator bool() const
{
    if (is_null())
        return 0;
    if (type!=Type::block || memberType.blockType!=BlockType::t_bool)
        throw ("Type-Error","Database::Element::operator bool");
    return fs->readAt<bool>(position);
}

Database::Element Database::Element::operator*() const
{
    if (is_null())
        return Element::null;
    if (type!=Type::ptr)
        throw Exception("Type-Error","Database::Element::operator*");
    unsigned int index = fs->readAt<unsigned int>(position);
    if (index==0)
        return Element::null;
    ClusterFile* ptr_cf = database->get_cluster(memberType.ptr_type);
    Element out;
    out.type = Type::cluster;
    out.fs = ptr_cf;
    out.position = index * memberType.ptr_type->size();
    out.database = database;
    return out;
}

std::unique_ptr<Database::Element> Database::Element::operator->() const
{
    std::unique_ptr<Element> out (new Element(**this));
    return out;
}

void Database::Element::operator= (int out)
{
    if (is_null())
        return;
    if (type!=Type::block || memberType.blockType!=BlockType::t_int)
        throw Exception("Type-Error","Database::Element::operator= int");
    fs->writeAt(position,out);
}

void Database::Element::operator= (char out)
{
    if (is_null())
        return;
    if (type!=Type::block || memberType.blockType!=BlockType::t_char)
        throw Exception("Type-Error","Database::Element::operator= char");
    fs->writeAt(position,out);
}

void Database::Element::operator= (const std::string& out)
{
    if (is_null())
        return;
    if (type!=Type::array || memberType.blockType!=BlockType::t_char)
        throw Exception("Type-Error","Database::Element::operator= std::string");
    for (std::streampos i=0;i<memberType.amount;i+=1)
    {
        if (i<out.size())
            fs->writeAt(position+i,out[i]);
        else
            fs->writeAt<char>(position+i,0);
    }
}

void Database::Element::operator= (double out)
{
    if (is_null())
        return;
    if (type!=Type::block || memberType.blockType!=BlockType::t_double)
        throw Exception("Type-Error","Database::Element::operator= double");
    fs->writeAt(position,out);
}

void Database::Element::operator= (bool out)
{
    if (is_null())
        return;
    if (type!=Type::block || memberType.blockType!=BlockType::t_bool)
        throw Exception("Type-Error","Database::Element::operator= bool");
    fs->writeAt(position,out);
}

void Database::Element::operator=(const Element& other)
{
    if (is_null())
        return;
    if (type!=Type::ptr)
        throw Exception("Type-Error","Database::Element::operator= Element");
    if (other.type==Type::null)
    {
        (*this)->add_refcount(-1);
        fs->writeAt(position,0);
        return;
    }
    if (other.type != Type::cluster)
        throw Exception ("Type-Error (Pointer muessen cluster zugewiesen werden)","Database::Element::operator= Element");
    if (*other.clusterType() != *memberType.ptr_type)
        throw Exception("Type-Error (Cluster Mismatch)","Database::Element::operator= Element");
    fs->writeAt(position,other.index());
    (*this)->add_refcount(1);
}

Database::Element Database::Element::operator() (const std::string& name) const
{
    if (is_null())
        return Element::null;
    if (type==Type::ptr)
        return (*this)->operator()(name);
    if (type!=Type::cluster)
        throw Exception("Type-Error","Database::Element::operator()");
    if (clusterType()->members.count(name)==0)
        throw Exception("Member not found","Database::Element:operator()");
    Element out;
    out.type = Type::block;
    out.fs = fs;
    out.position = position + clusterType()->offsetOf(name);
    out.database = database;
    out.memberType = clusterType()->members.at(name);

    if (out.memberType.amount>1)
        out.type = Type::array;
    if (out.memberType.blockType == BlockType::t_ptr)
        out.type = Type::ptr;
    if (out.memberType.blockType == BlockType::t_list)
        out.type = Type::list;
    return out;
}

unsigned int Database::Element::index() const
{
    if (is_null())
        return 0;
    if (type!=Type::cluster)
        throw Exception("Type-Error","Database::Element::index()");
    return position / clusterType()->size();
}

void Database::Element::erase()
{
    if (is_null())
        return;
    if (type!=Type::cluster)
        throw Exception("Type-Error","Database::Element::erase()");
    clusterFile()->erase(position);
    type = Type::null;
}

unsigned int Database::Element::reference_count()
{
    if (is_null())
        return 0;
    if (type!=Type::cluster)
        throw Exception("Type-Error","Database::Element::reference_count()");
    return clusterFile()->get_refcount(position);
}

void Database::Element::clear_references()
{
    if (is_null())
        return;
    if (type!=Type::cluster)
        throw Exception("Type-Error","Database::Element::clear_references()");
    for (auto& item: clusterType()->members)
    {
        if (item.second.blockType==BlockType::t_ptr) {
            (*this)(item.first) = Element::null;
        }
        if (item.second.blockType==BlockType::t_list) {
            Element list = (*this)(item.first);
            while (!is_null() && list.begin()!=list.end())
                list.erase(list.begin());
        }
    }
}

unsigned int Database::Element::size() const
{
    if (is_null())
        return 0;
    if (type!=Type::array)
        throw Exception("Type-Error","Database::Element::size()");
    return memberType.amount;
}

Database::Element Database::Element::operator[] (unsigned int index)
{
    if (is_null())
        return Element::null;
    if (type==Type::clusterList)
    {
        return clusterFile()->at(index * clusterType()->size());
    }
    else if (type==Type::array)
    {
        if (index>=memberType.amount)
            return Element::null;
        Element out;
        out.type = Type::block;
        out.fs = fs;
        out.position = position + index*memberType.blockType.size;
        out.database = database;
        out.memberType = memberType;
        out.memberType.amount=0;
        return out;
    }
    throw Exception("Type-Error","Database::Element::operator[]");
}

const Database::Element Database::Element::operator[] (unsigned int index) const
{
    return const_cast<Element*>(this)->operator[](index);
}

Database::Iterator Database::Element::begin()
{
    if (is_null())
        return end();
    if (type==Type::clusterList)
    {
        Iterator out;
        out.parent.copy(*this);
        out.ref.type = Type::cluster;
        out.ref.fs = fs;
        out.ref.position = clusterFile()->get_first_filled();
        out.ref.database = database;

        if (out.ref.position==0)
            return end();

        return out;
    }
    else if (type==Type::list)
    {
        Iterator out;
        out.parent.copy(*this);
        out.ref.type = Type::ptr;
        out.ref.fs = &database->listfile;
        out.ref.position = database->listfile.ptr_pos(fs->readAt<unsigned int>(position) * ListFile::LINESIZE);
        out.ref.database = database;
        out.ref.memberType.amount = 1;
        out.ref.memberType.blockType = BlockType::t_ptr;
        out.ref.memberType.ptr_type = memberType.ptr_type;
        if (out.ref.position < ListFile::LINESIZE)
            return end();
        return out;
    }
    else if (type==Type::array)
    {
        Iterator out;
        out.parent.copy(*this);
        out.ref.copy((*this)[0]);
        return out;
    }
    
    throw Exception("Type-Error","Database::Element::begin()");
}

Database::Iterator Database::Element::end()
{
    return Iterator();
}

Database::const_Iterator Database::Element::begin() const
{
    return cbegin();
}

Database::const_Iterator Database::Element::end() const
{
    return cend();
}

Database::const_Iterator Database::Element::cbegin() const
{
    return const_cast<Element*>(this)->begin();
}

Database::const_Iterator Database::Element::cend() const
{
    return const_cast<Element*>(this)->end();
}

Database::Element Database::Element::emplace()
{
    if (is_null())
        return Element::null;
    else if (type==Type::list)
    {
        std::streampos new_item = database->listfile.remove_empty();
        std::streampos first = fs->readAt<unsigned int>(position) * ListFile::LINESIZE;
        if (first!=0)
            database->listfile.set_previous(first,new_item);
        fs->writeAt<unsigned int>(position,new_item/ListFile::LINESIZE);
        database->listfile.set_next(new_item,first);
        database->listfile.set_previous(new_item,0);
        Element out = *(this->begin());
        out.fs->writeAt<unsigned int>(out.position,0);
        return out;
    }
    else if (type==Type::clusterList)
    {
        Element out;
        out.type = Type::cluster;
        out.fs = fs;
        out.position = clusterFile()->emplace();
        out.database = database;
        if (!database->is_good())
            return null;
        out.cluster_init();
        return out;
    }

    throw Exception("Type-Error","Database::Element::emplace()");
}

void Database::Element::erase(const const_Iterator& it)
{
    if (is_null())
        return;
    if (type!=Type::list)
        throw Exception("Type-Error","Database::Element::erase(iterator)");
    if (it.it.parent != *this)
        throw Exception("Iterator-mismatch","Database::Element::erase()");
    *(it.it) = Element::null;
    std::streampos to_remove = it.it->position - (it.it->position % ListFile::LINESIZE);
    std::streampos next = database->listfile.get_next(to_remove);
    std::streampos previous = database->listfile.get_previous(to_remove);
    if (next!=0)
        database->listfile.set_previous(next,previous);
    if (previous!=0)
        database->listfile.set_next(previous,next);
    else
        fs->writeAt<unsigned int>(position,next/ListFile::LINESIZE);
    database->listfile.append_empty(to_remove);
}

const Database::Element Database::Element::null;

Database::ClusterType* Database::Element::clusterType()
{
    return &clusterFile()->info;
}

const Database::ClusterType* Database::Element::clusterType() const
{
    return &clusterFile()->info;
}

Database::ClusterFile* Database::Element::clusterFile()
{
    return static_cast<ClusterFile*>(fs);
}

const Database::ClusterFile* Database::Element::clusterFile() const
{
    return static_cast<const ClusterFile*>(fs);
}

void Database::Element::add_refcount(int add)
{
    if (is_null())
        return;
    if (type!=Type::cluster)
        throw Exception("implementierungsfehler","Database::Element::add_refcount()");
    clusterFile()->set_refcount_add(position,add);
}

void Database::Element::cluster_init()
{
    if (is_null())
        return;
    if (type!=Type::cluster)
        throw Exception("implementierungsfehler","Database::Element::cluster_init()");
    for (auto& it: clusterType()->members) {
        if (it.second.blockType==BlockType::t_ptr) {
            Element ptr = (*this)(it.first);
            ptr.fs->writeAt<unsigned int>(ptr.position,0);
        }
        if (it.second.blockType==BlockType::t_list) {
            Element list = (*this)(it.first);
            list.fs->writeAt<unsigned int>(list.position,0);
        }
    }
}

Database::Iterator& Database::Iterator::operator++()
{
    if (ref.is_null())
        return *this;
    switch (parent.type) {
        case Element::Type::clusterList:
            ref.position = ref.clusterFile()->get_next(ref.position);
            if (ref.position==0)
            {
                ref.copy(Element::null);
                parent.copy(Element::null);
            }
            return *this;
        case Element::Type::array:
            ref.position += ref.memberType.blockType.size;
            if (ref.position>=parent.position+parent.memberType.size())
            {
                ref.copy(Element::null);
                parent.copy(Element::null);
            }
            return *this;
        case Element::Type::list:
            {
                std::streampos my_item = ref.position - (ref.position % ListFile::LINESIZE);
                ref.position = ref.database->listfile.ptr_pos(ref.database->listfile.get_next(my_item));
                if (ref.position < ListFile::LINESIZE)
                {
                    ref.copy(Element::null);
                    parent.copy(Element::null);
                }
            }
            return *this;
        default:
            throw Exception("Ungueltiger Iterator","Database::Iterator::operator++");
    }
}

Database::Iterator Database::Iterator::operator++(int)
{
    Iterator out = *this;
    ++*this;
    return out;
}

bool Database::Iterator::operator==(const Iterator& other) const
{
    return ref == other.ref;
}

bool Database::Iterator::operator!=(const Iterator& other) const
{
    return !(*this==other);
}

Database::Element Database::Iterator::operator*() const
{
    return ref;
}

Database::Element* Database::Iterator::operator->() const
{
    return &ref;
}

Database::Database(): listfile(this), structurefile(this), status(Status::empty)
{ }

Database::Database(const FileName& _path): listfile(this), structurefile(this), status(Status::empty), path(_path)
{ }

void Database::setPath(const FileName& fname)
{
    path = fname;
}

bool Database::init()
{
    if (status!=Status::empty)
        throw Exception("Die Datenbank ist bereits geladen","Database::init()");
    structurefile.name = path+"struktur.txt";
    listfile.name = path+"lists.data";
    parse_structure();
    structurefile.close();
    if (!is_good())
        return false;
    for (auto& cf: clusters) {
        cf.name = path+(cf.info.name+".data");
    }
    status = Status::closed;
    return true;
}

bool Database::is_init() const
{
    return status==Status::closed || status==Status::open;
}

bool Database::open()
{
    if (status!=Status::closed)
        return false;
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

Database::Element Database::list(const std::string& name)
{
    if (!is_good())
        return Element::null;
    ClusterFile* cluster = get_cluster(name);
    if (cluster==nullptr)
        throw Exception("Ungueltiger Typenname","Database::list()");
    return cluster->list();
}

const Database::Element Database::list(const std::string& name) const
{
    return const_cast<Database*>(this)->list(name);
}

Database::File::File(Database* db): database(db)
{ }

void Database::File::open()
{
    fs.close();
    fs.open(name.fullName(),fs.in|fs.out);
    if (fs.good())
        return;
    fs.close();
    fs.open(name.fullName(),fs.out);
    if (!fs.good()) {
        database->status = Status::error;
        database->log << name.fullName() << " konnte nicht geoeffnet werden" << std::endl;
        return;
    }
    database->log << name.fullName() << " wurde neu erstellt" << std::endl;
    fs.close();
    fs.open(name.fullName(),fs.in|fs.out);
    if (!fs.good())
    {
        database->status = Status::error;
        database->log << name.fullName() << " kann nicht gelesen werden" << std::endl;
        return;
    }
}

void Database::File::close()
{
    fs.close();
}

std::streampos Database::File::size() const
{
    fs.seekg(0,fs.end);
    std::streampos len = fs.tellg();
    if (!fs.good()) {
        database->status = Status::error;
        database->log << "Fehler bei Ermittlung der Dateigroesse: (" << name.fileOnly() << ")" << std::endl;
        return 0;
    }
    return len;
}

template<class PrimT>
PrimT Database::File::readAt(std::streampos where) const
{
    if (!database->is_good())
        return 0;
    fs.seekg(where);
    PrimT out;
    if (!serial_read(fs,out)) {
        database->status = Status::error;
        database->log << "Lesefehler an Position " << fs.tellg() << " (" << name.fileOnly() << ")" << std::endl;
        return 0;
    }
    return out;
}

template <class PrimT>
void Database::File::writeAt(std::streampos where, PrimT what)
{
    if (!database->is_good())
        return;
    fs.seekp(where);
    if (!serial_print(fs,what)) {
        database->status = Status::error;
        database->log << "Schreibfehler an Position " << fs.tellp() << " (" << name.fileOnly() << ")" << std::endl;
    }
}

Database::ClusterFile::ClusterFile(Database* db): File(db)
{ }

void Database::ClusterFile::open()
{
    File::open();
    std::streampos len = size();
    if (len==0)
    {
        fs << std::string(info.size()-2*serial_size<char>(),0);
        fs << "\r\n";
        set_first_empty(0);
        set_last_empty(0);
        set_first_filled(0);
        set_last_filled(0);
    }
}

std::streampos Database::ClusterFile::get_first_filled() const
{
    unsigned int index = readAt<unsigned int>(0);
    return info.size() * index;
}

std::streampos Database::ClusterFile::get_first_empty() const
{
    unsigned int index = readAt<unsigned int>(2*serial_size<unsigned int>());
    return info.size() * index;
}

std::streampos Database::ClusterFile::get_last_filled() const
{
    unsigned int index = readAt<unsigned int>(1*serial_size<unsigned int>());
    return info.size() * index;
}

std::streampos Database::ClusterFile::get_last_empty() const
{
    unsigned int index = readAt<unsigned int>(3*serial_size<unsigned int>());
    return info.size() * index;
}

std::streampos Database::ClusterFile::get_next(std::streampos where) const
{
    where += serial_size<bool>() + serial_size<unsigned int>();
    unsigned int index = readAt<unsigned int>(where);
    return info.size() * index;
}

std::streampos Database::ClusterFile::get_previous(std::streampos where) const
{
    where += serial_size<bool>();
    unsigned int index = readAt<unsigned int>(where);
    return info.size() * index;
}

bool Database::ClusterFile::get_occupied(std::streampos where) const
{
    return readAt<bool>(where);
}

unsigned int Database::ClusterFile::get_refcount(std::streampos where) const
{
    where += serial_size<bool>() + 2*serial_size<unsigned int>();
    return readAt<unsigned int>(where);
}

void Database::ClusterFile::set_first_filled(std::streampos what)
{
    unsigned int index = what/info.size();
    writeAt(0,index);
}

void Database::ClusterFile::set_first_empty(std::streampos what)
{
    unsigned int index = what/info.size();
    writeAt(2*serial_size<unsigned int>(),index);
}

void Database::ClusterFile::set_last_filled(std::streampos what)
{
    unsigned int index = what/info.size();
    writeAt(1*serial_size<unsigned int>(),index);
}

void Database::ClusterFile::set_last_empty(std::streampos what)
{
    unsigned int index = what/info.size();
    writeAt(3*serial_size<unsigned int>(),index);
}

void Database::ClusterFile::set_next(std::streampos where, std::streampos what)
{
    where += serial_size<bool>() + serial_size<unsigned int>();
    unsigned int index = what/info.size();
    writeAt(where,index);
}

void Database::ClusterFile::set_previous(std::streampos where, std::streampos what)
{
    where += serial_size<bool>();
    unsigned int index = what/info.size();
    writeAt(where,index);
}

void Database::ClusterFile::set_occupied(std::streampos where, bool what)
{
    writeAt(where,what);
}

void Database::ClusterFile::clear_refcount(std::streampos where)
{
    where += serial_size<bool>()+2*serial_size<unsigned int>();
    writeAt<unsigned int>(where,0);
}

void Database::ClusterFile::set_refcount_add(std::streampos where, int add)
{
    unsigned int refcount = get_refcount(where);
    refcount+=add;
    where += serial_size<bool>() + 2*serial_size<unsigned int>();
    writeAt(where,refcount);
}

std::streampos Database::ClusterFile::extend()
{
    if (!database->is_good())
        return size();
    fs.seekp(0,fs.end);
    std::streampos new_space = fs.tellp();
    fs << std::string(info.size() - 2*serial_size<char>(), 0);
    fs << "\r\n";
    if (!fs.good()) {
        database->status = Status::error;
        database->log << "Datei konnte nicht erweitert werden: " << name.fileOnly() << std::endl;
    }
    return new_space;
}

std::streampos Database::ClusterFile::remove_empty()
{
    std::streampos first = get_first_empty();
    if (first == 0)
        return extend();
    std::streampos next = get_next(first);
    set_first_empty(next);
    if (next==0)
        set_last_empty(0);
    return first;
}

void Database::ClusterFile::append_empty(std::streampos new_empty)
{
    std::streampos last = get_last_empty();
    if (last==0)
        set_first_empty(new_empty);
    else
        set_next(last,new_empty);
    set_last_empty(new_empty);
}

void Database::ClusterFile::append_filled(std::streampos new_filled)
{
    std::streampos last = get_last_filled();
    if (last==0)
        set_first_filled(new_filled);
    else
        set_next(last,new_filled);
    set_last_filled(new_filled);
}

void Database::ClusterFile::remove_filled(std::streampos to_remove)
{
    std::streampos next = get_next(to_remove);
    std::streampos previous = get_previous(to_remove);
    if (next==0)
        set_last_filled(previous);
    else
        set_previous(next,previous);
    if (previous==0)
        set_first_filled(next);
    else
        set_next(previous,next);
}

std::streampos Database::ClusterFile::emplace()
{
    std::streampos new_filled = remove_empty();
    append_filled(new_filled);
    set_occupied(new_filled,true);
    return new_filled;
}

void Database::ClusterFile::erase(std::streampos to_remove)
{
    if (get_refcount(to_remove)>0)
        throw Exception("Removal of referenced Database-Element","Database::ClusterFile::erase");
    remove_filled(to_remove);
    append_empty(to_remove);
    at(to_remove).clear_references();
    set_occupied(to_remove,false);
}

Database::Element Database::ClusterFile::list()
{
    Element out;
    out.type = Element::Type::clusterList;
    out.fs = this;
    out.position=0;
    out.database=database;
    return out;
}

const Database::Element Database::ClusterFile::list() const
{
    if (!database->is_good())
        return Database::Element::null;
    return const_cast<ClusterFile*>(this)->list();
}

Database::Element Database::ClusterFile::at(std::streampos where)
{
    if (where==0)
        return Element::null;
    if (where>=size())
        return Element::null;
    Element out;
    out.type = Element::Type::cluster;
    out.fs = this;
    out.position = where;
    out.database = database;
    return out;
}

Database::ListFile::ListFile(Database* db): File(db) {};

void Database::ListFile::open()
{
    File::open();
    std::streampos len = size();
    if (len==0) {
        fs << std::string(LINESIZE-2*serial_size<char>(),0);
        fs << "\r\n";
        set_first_empty(0);
        set_last_empty(0);
    }
}

std::streampos Database::ListFile::get_first_empty() const
{
    unsigned int index = readAt<unsigned int>(0);
    return index*LINESIZE;
}

std::streampos Database::ListFile::get_last_empty() const
{
    std::streampos where = 1*serial_size<unsigned int>();
    unsigned int index = readAt<unsigned int>(where);
    return index*LINESIZE;
}

std::streampos Database::ListFile::get_next(std::streampos where) const
{
    where += 1*serial_size<unsigned int>();
    unsigned int index = readAt<unsigned int>(where);
    return index*LINESIZE;
}

std::streampos Database::ListFile::get_previous(std::streampos where) const
{
    unsigned int index = readAt<unsigned int>(where);
    return index*LINESIZE;
}

void Database::ListFile::set_first_empty(std::streampos what)
{
    unsigned int index = what/LINESIZE;
    writeAt(0,index);
}

void Database::ListFile::set_last_empty(std::streampos what)
{
    unsigned int index = what/LINESIZE;
    std::streampos where = serial_size<unsigned int>();
    writeAt(where,index);
}

void Database::ListFile::set_next(std::streampos where, std::streampos what)
{
    unsigned int index = what/LINESIZE;
    where += serial_size<unsigned int>();
    writeAt(where,index);
}

void Database::ListFile::set_previous(std::streampos where, std::streampos what)
{
    unsigned int index = what/LINESIZE;
    writeAt(where,index);
}

std::streampos Database::ListFile::extend()
{
    if (!database->is_good())
        return 0;
    fs.seekp(0,fs.end);
    std::streampos new_space = fs.tellp();
    fs << std::string(LINESIZE-2*serial_size<char>(),0);
    fs << "\r\n";
    if (!fs.good()) {
        database->status = Status::error;
        database->log << "Fehler beim Erweitern der Listendatei" << std::endl;
        return 0;
    }
    return new_space;
}

std::streampos Database::ListFile::remove_empty()
{
    std::streampos first = get_first_empty();
    if (first==0)
        return extend();
    std::streampos next = get_next(first);
    set_first_empty(next);
    if (next==0)
        set_last_empty(0);
    return first;
}

void Database::ListFile::append_empty(std::streampos where)
{
    std::streampos last = get_last_empty();
    if (last==0)
        set_first_empty(where);
    else
        set_next(last,where);
    set_last_empty(where);
}

std::streampos Database::ListFile::ptr_pos(std::streampos where) const
{
    return where + 2*serial_size<unsigned int>();
}

// linesize: | previous | next | ref | "\r\n" |
const std::streampos Database::ListFile::LINESIZE = 3*serial_size<unsigned int>() + 2;

const std::string Database::valid_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-";

Database::ClusterFile* Database::get_cluster(const std::string& name)
{
    for (auto& cf: clusters) {
        if (cf.info.name==name)
            return &cf;
    }
    return nullptr;
}

Database::ClusterFile* Database::get_cluster(const ClusterType* cltype)
{
    for (auto& cf: clusters) {
        if (&cf.info == cltype)
            return &cf;
    }
    throw Exception("Clustertype ohne Clusterfile","Database::get_cluster(ClusterType*)");
}

bool Database::valid_name(const std::string& name)
{
    if (name == "int" || name == "char" || name == "double" || name == "bool")
        return false;
    for (char c: name)
        if (valid_chars.find(c)==std::string::npos)
            return false;
    return true;
}

void Database::parse_structure()
{
    structurefile.open();
    parse_typenames();
    structurefile.close();
    if (!is_good())
        return;
    structurefile.open();
    while (parse_check("type"))
        parse_cluster();
    if (!is_good())
        return;
    if (structurefile.fs.good()) {
        parse_errorlog("Es wird \"type\" erwartet");
    }
}

bool Database::parse_check(const std::string& str)
{
    std::streampos start = structurefile.fs.tellg();
    std::string next;
    structurefile.fs>>next;
    structurefile.fs.seekg(start);
    return next==str;
}

void Database::parse_typenames()
{
    if (!is_good())
        return;
    std::string command;
    while (structurefile.fs >> command)
    {
        if (command!="type")
            continue;
        structurefile.fs >> command;
        if (!structurefile.fs.good())
        {
            parse_errorlog("Es wird ein Name erwartet");
            return;
        }
        if (!valid_name(command))
        {
            parse_errorlog("Unzulaessiger name");
            return;
        }
        if (get_cluster(command)!=nullptr)
        {
            parse_errorlog("doppelter Typenname");
            return;
        }
        clusters.emplace_back(this);
        clusters.back().info.name=command;
    }
}

void Database::parse_cluster()
{
    if (!is_good())
        return;
    std::string name;
    // "type" überspringen
    structurefile.fs >> name >> name;
    ClusterFile* cluster = get_cluster(name);
    if (cluster==nullptr)
        throw Exception("Implementierungsfehler","Database::parse_cluster()");
    while (is_good())
    {
        if (parse_check("type"))
            return;
        if (!structurefile.fs.good())
            return;
        parse_block(cluster->info);
    }
}

void Database::parse_block(ClusterType& cluster)
{
    if (!is_good())
        return;
    std::string type;
    MemberType member;
    if (!(structurefile.fs >> type))
    {
        parse_errorlog("Dateiende - Es wird ein BlockTyp erwartet");
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
        ClusterFile* cf = get_cluster(type);
        if (cf==nullptr)
        {
            parse_errorlog("Unbekannter Membertyp");
            return;
        }
        member.ptr_type = &(cf->info);
    }

    parse_arr_len(member.amount);

    if (!is_good())
        return;

    std::string name;
    if (!(structurefile.fs >> name)) {
        parse_errorlog("Name erwartet");
        return;
    }

    if (name=="[]") {
        if (member.blockType!=BlockType::t_ptr) {
            parse_errorlog("Listen existieren nur fuer Referenzen");
            return;
        }
        if (member.amount!=1) {
            parse_errorlog("Eine liste kann nur einfach vorhanden sein");
            return;
        }
        member.blockType = BlockType::t_list;
        if (!(structurefile.fs >> name)) {
            parse_errorlog("Name erwartet");
            return;
        }
    }

    if (!valid_name(name)) {
        parse_errorlog("ungueltiger Name");
        return;
    }

    if (cluster.members.count(name)>0) {
        parse_errorlog("doppelter Membername");
        return;
    }

    cluster.members[name] = member;
    return;
}

void Database::parse_arr_len(unsigned int& out)
{
    if (!is_good())
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
        parse_errorlog("Die Anzahl muss groesser als 0 sein");
        return;
    }
}

void Database::parse_errorlog(const std::string& msg)
{
    log << "struktur.txt Position " << structurefile.fs.tellg() << ": " << msg << std::endl;
    status = Status::error;
}