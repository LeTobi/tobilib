#include "filename.h"

namespace tobilib {
    FileName::FileName() {
    }

    FileName::FileName(const StringPlus& filestring) {
        assign(filestring);
    }

    FileName::FileName(const std::string& filestring) {
        assign(filestring);
    }

    FileName::FileName(const char* filestring) {
        assign(filestring);
    }

    void FileName::assign(const StringPlus& filestring) {
        while (!path.empty())
            path.pop_back();

        // Länge von frags ist mindestens 1
        std::vector<StringPlus> frags = filestring.split_all_of("\\/");
        
        // Dateiname extrahieren
        if (frags.back().count_all(".")>0)
            extension = frags.back().split(".").back();
        int extstart = frags.back().rfind(".");
        if (extstart==StringPlus::npos)
            extstart = frags.back().size();
        name = frags.back().substr(0,extstart);
        frags.pop_back();

        // Pfad extrahieren
        for (auto& frag: frags)
            path.push_back(frag);
        optimize();
    }

    std::istream& operator>> (std::istream& in, tobilib::FileName& fname) {
        std::string str;
        in >> str;
        fname.assign(str);
        return in;
    }

    void FileName::extend (const FileName& other) {
        path.insert(path.end(),other.path.begin(),other.path.end());
        name = other.name;
        extension = other.extension;
        optimize();
    }

    FileName& FileName::operator<< (const FileName& other) {
        extend(other);
        return *this;
    }

    FileName FileName::operator+ (const FileName& other) const {
        return FileName(*this) << other;
    }

    FileName FileName::operator+ (const StringPlus& path) const {
        return FileName(*this) << path;
    }

    FileName FileName::operator+ (const std::string& path) const {
        return FileName(*this) << path;
    }

    FileName FileName::operator+ (const char* path) const {
        return FileName(*this) << path;
    }

    StringPlus FileName::fileOnly() const {
        StringPlus out = name;
        if (!extension.empty()) {
            out+=".";
            out+=extension;
        }
        return out;
    }

    StringPlus FileName::directory() const {
        StringPlus out;
        for (auto& frag: path) {
            out+=frag+"/";
        }
        return out;
    }

    StringPlus FileName::fullName() const {
        return directory()+fileOnly();
    }

    std::ostream& operator<< (std::ostream& out, const tobilib::FileName& fname) {
        out << fname.fullName();
        return out;
    }

    bool FileName::operator==(const FileName& other) const {
        return fullName() == other.fullName();
    }

    bool FileName::contains(const FileName& child) const {
        return child.fullName().beginsWith(fullName());
    }

    void FileName::optimize() {
        auto frag=path.begin();
        while (frag!=path.end()) {
            if (*frag==".") {
                frag = path.erase(frag);
            }
            else if (*frag==".." && frag!=path.begin()) {
                frag = path.erase(--frag);
                frag = path.erase(frag);
            }
            else {
                frag++;
            }
        }
    }
}