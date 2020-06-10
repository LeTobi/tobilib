#ifndef TC_FILENAME_H
#define TC_FILENAME_H

#include "stringplus.h"
#include <list>
#include <iostream>

namespace tobilib
{
    class FileName {
    public:
        std::list<StringPlus> path;
        StringPlus name;
        StringPlus extension;

        FileName();
        FileName(const StringPlus&);
        FileName(const std::string&);
        FileName(const char*);

        void assign(const StringPlus&);
        FileName& operator= (StringPlus& other){assign(other);return *this;};
        friend std::istream& operator>> (std::istream&, tobilib::FileName&);

        void extend(const FileName&);
        FileName& operator<< (const FileName&);
        FileName operator+ (const FileName&) const;
        FileName operator+ (const StringPlus&) const;
        FileName operator+ (const std::string&) const;
        FileName operator+ (const char*) const;

        StringPlus fileOnly() const;
        StringPlus directory() const;
        StringPlus fullName() const;
        friend std::ostream& operator<< (std::ostream&, const tobilib::FileName&);

        bool operator== (const FileName&) const;
        bool contains (const FileName&) const;

        const char* c_str() const {return fullName().toString().c_str();};
        operator StringPlus () const {return fullName();};
        
    private:
        void optimize();
    };
}

#endif