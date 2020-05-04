#include "database.h"

using namespace tobilib::database;

Element::Element() : type(Type::null)
{ }

Element Element::null = Element();

Element::Type Element::get_type() const
{
    return type;
}

bool Element::is_null() const
{
    return type==Type::null;
}

const Element Element::operator[] (unsigned int idx) const
{
    if (!sanityCheck())
        return Element::null;
    if (type==Type::cluster_file)
    {
        if (idx>=size())
            throw Exception("Out of range","database::Element::operator[]");
        Element out (Type::cluster,database);
        out.cluster_info = cluster_file_info;
        out.fs = fs;
        out.position = idx*Access::clusterSize(*cluster_file_info);
        return out;
    }
    else if (type==Type::array)
    {
        if (idx>=size())
            throw Exception("Out of range","database::Element::operator[]");
        Element out (Type::block,database);
        out.fs = fs;
        out.block_info = array_info;
        out.position = position + (Access::primitiveSize(*array_info)*idx);
        return out;
    }
    else if (type==Type::block && block_info->type==BlockType::t_list_ptr)
    {
        //TODO
    }
    else
    {
        throw Exception("type_error","database::Element::operator[]");
    }
}

Element Element::operator[] (unsigned int index)
{
    Element result = (*const_cast<const Element*>(this))[index];
    return *const_cast<Element*>(&result);
}

unsigned int Element::size() const
{
    if (!sanityCheck())
        return 0;
    if (type==Type::cluster_file)
    {
        fs->seekg(0);
        std::streampos start = fs->tellg();
        fs->seekg(0,fs->end);
        std::streampos end = fs->tellg();
        fs->seekg(0);
        if (!fs->good())
        {
            setError("Zugriffsfehler");
            return 0;
        }
        return (end-start)/Access::clusterSize(*cluster_file_info);
    }
    else if (type==Type::array)
    {
        return array_info->amount;
    }
    else if (type==Type::block && block_info->type==BlockType::t_list_ptr)
    {
        //TODO
    }
    else
    {
        throw Exception("type_error","database::Element::size()");
    }
}

const Iterator Element::begin() const
{
    //TODO
}

Iterator Element::begin()
{
    const Iterator result = const_cast<const Element*>(this)->begin();
    return *(const_cast<Iterator*>(&result));
}

const Iterator Element::end() const
{
    //TODO
}

Iterator Element::end()
{
    const Iterator result = const_cast<const Element*>(this)->end();
    return *(const_cast<Iterator*>(&result));
}


Element Element::emplace_back()
{
    if (!sanityCheck())
        return Element::null;
    if (type==Type::cluster_file)
    {
        unsigned int linesize = Access::clusterSize(*cluster_file_info);
        for (unsigned int i=0;i<size();i++)
        {
            Element out = (*this)[i];
            if (!out.is_free())
                continue;
            database->initialize(*cluster_file_info,i);
            return out;
        }
        database->initialize(*cluster_file_info,size());
        return *this[size()-1];
    }
    else if (type==Type::block && block_info->type==BlockType::t_list_ptr)
    {
        //TODO
    }
    else
    {
        throw Exception("type_error","database::Element::emplace_back()");
    }
}

void Element::remove(const Element&)
{
    //TODO
}

void Element::insert(const Element& element, const Element& successor)
{
    //TODO
}

void Element::clear()
{
    //TODO
}

Element::operator int() const
{
    if (type!=Type::block || block_info->type!=BlockType::t_int)
        throw Exception("type_error","database::Element::operator int()");
    if (!sanityCheck())
        return 0;
    char in [sizeof(int)];
    fs->seekg(position);
    for (unsigned int i=0;i<sizeof(int);i++) {
        in[i] = fs->get();
    }
    if (!fs->good()) {
        setError("Zugriffsfehler");
        return 0;
    }
    return *reinterpret_cast<int*>(in);
}

void Element::operator= (int output_int)
{
    if (type!=Type::block || block_info->type!=BlockType::t_int)
        throw Exception("type_error","database::Element::operator=(int)");
    if (!sanityCheck())
        return;
    char* c = reinterpret_cast<char*>(&output_int);
    fs->seekp(position);
    for (unsigned int i=0;i<sizeof(int);i++)
        fs->put(*(c+i));
    if (!fs->good())
        setError("Zugriffsfehler");
}

Element::operator char() const 
{
    if (type!=Type::block || block_info->type!=BlockType::t_char)
        throw Exception("type_error","database::Element::operator char()");
    if (!sanityCheck())
        return 0;
    fs->seekg(position);
    char out = fs->get();
    if (!fs->good()) {
        setError("Zugriffsfehler");
        return 0;
    }
    return out;
}

void Element::operator= (char output_char)
{
    if (!sanityCheck())
        return;
    fs->seekp(position);
    fs->put(output_char);
    if (!fs->good()){
        setError("Zugriffsfehler");
        return;
    }
}

Element::operator double() const
{
    if (type!=Type::block || block_info->type!=BlockType::t_decimal)
        throw Exception("type_error","database::Element::operator double()");
    if (!sanityCheck())
        return 0;
    char out [sizeof(double)];
    fs->seekg(position);
    for (unsigned int i=0;i<sizeof(double);i++)
        out[i] = fs->get();
    if (!fs->good()) {
        setError("Zugriffsfehler");
        return 0;
    }
    return *reinterpret_cast<double*>(out);
}

void Element::operator= (double output_decimal)
{
    if (type!=Type::block || block_info->type!=BlockType::t_decimal)
        throw Exception("type_error","database::Element::operator=(double)");
    if (!sanityCheck())
        return;
    char* cptr = reinterpret_cast<char*>(&output_decimal);
    fs->seekp(position);
    for (unsigned int i=0;i<sizeof(double);i++)
        fs->put(*(cptr+i));
    if (!fs->good())
        setError("Zugriffsfehler");
}

Element::operator bool() const
{
    if (type!=Type::block || block_info->type!=BlockType::t_bool)
        throw Exception("type_error","database::Element::operator bool()");
    if (!sanityCheck())
        return false;
    fs->seekp(position);
    bool out = fs->get()=='1';
    if (!fs->good()) {
        setError("Zugriffsfehler");
        return false;
    }
    return out;
}

void Element::operator= (bool output_bool)
{
    if (type!=Type::block || block_info->type!=BlockType::t_bool)
        throw Exception("type_error","database::Element::operator=(bool)");
    if (!sanityCheck())
        return;
    fs->seekp(position);
    fs->put(output_bool?'1':'0');
    if (!fs->good())
        setError("Zugriffsfehler");
}

const Element Element::operator->() const
{
    //TODO
}

const Element Element::operator*() const
{
    //TODO
}

Element Element::operator->()
{
    const Element result = (*const_cast<const Element*>(this)).operator->();
    return *const_cast<Element*>(&result);
}

Element Element::operator*()
{
    const Element result = *(*const_cast<const Element*>(this));
    return *const_cast<Element*>(&result);
}

const Element Element::operator()(const std::string& name) const
{
    if (type==Type::block && block_info->type==BlockType::t_cluster_ptr)
        return (*this)(name);
    if (type != Type::cluster)
        throw Exception("type_error","database::Element::member()");
    Element out (Type::block, database);
    out.fs = fs;
    std::streampos offset = 1; // occupation
    for (auto& block: cluster_info->segmentation) {
        if (block.name==name)
        {
            if (block.amount>1) {
                out.type = Type::array;
                out.array_info = &block;
            }
            else {
                out.block_info = &block;
            }
            out.position = position+offset;
            return out;
        }
        else
        {
            offset += Access::blockSize(block);
        }
    }

    throw Exception("Member existiert nicht","database::Element::member()");
}

Element Element::operator()(const std::string& name)
{
    const Element result = (*const_cast<const Element*>(this))(name);
    return *const_cast<Element*>(&result);
}

unsigned int Element::index() const
{
    if (type==Type::block && block_info->type==BlockType::t_cluster_ptr)
        return this->index();
    if (type!=Type::cluster)
        throw Exception("type_error","database::Element::index()");
    return position/Access::clusterSize(*cluster_info);
}

bool Element::is_free() const
{
    if (type==Type::block && block_info->type==BlockType::t_cluster_ptr)
        return this->is_free();
    if (type!=Type::cluster)
        throw Exception("type_error","database::Element::is_free()");
    fs->seekp(position);
    bool out = fs->get()==' ';
    if (!fs->good())
    {
        setError("Zugriffsfehler");
        return false;
    }
    return out;
}

Element::operator std::string() const 
{
    if (type!=Type::array || array_info->type!=BlockType::t_char)
        throw Exception("type_error","database::Element::operator string()");
    if (!sanityCheck())
        return "";
    std::string out;
    fs->seekg(position);
    for (unsigned int i=0;i<size();i++) {
        char c = fs->get();
        if (c==0)
            break;
        out+=c;
    }
    if (!fs->good()) {
        setError("Zugriffsfehler");
        return "";
    }
    return out;
}

void Element::operator=(const std::string& str)
{
    if (type!=Type::array || array_info->type!=BlockType::t_char)
        throw Exception("type_error","database::Element::operator=(string)");
    if (!sanityCheck())
        return;
    if (str.size()>array_info->amount)
        throw Exception("oversize","database::Element::operator=(string)");
    fs->seekp(position);
    *fs << str << std::string(array_info->amount-str.size(),0);
    if (!fs->good())
        setError("Zugriffsfehler");
}

void Element::operator=(const char* str)
{
    *this = std::string(str);
}

bool Element::operator== (const Element&) const
{
    //TODO
}

Element::Element(Type t, Access* accptr): type(t), database(accptr)
{ }

void Element::setError(const std::string& msg) const
{
    database->state = Access::State::error;
    database->log << msg;
}

bool Element::sanityCheck() const
{
    return database->is_open();
}