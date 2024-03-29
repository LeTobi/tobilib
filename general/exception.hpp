#ifndef TC_EXCEPTION_H
#define TC_EXCEPTION_H

#include <exception>
#include <string>
#include <list>
#include <ctime>
#include <iostream>

namespace tobilib
{
    class Exception: public std::exception
    {
    private:
        // what_str: wird verwendet um eine c-string ausgabe zu generieren e.g. what()
        //  mutable weil nichts am status verändert wird.
        mutable std::string what_str;

    public:
        time_t creation;
        std::string msg;
        std::list<std::string> trace;

        template <class strT>
        Exception(strT txt): msg(txt)
        {
            time(&creation);
        };

        template <class strT>
        Exception(strT txt, std::string first_trace): msg(txt) {
            time(&creation);
            trace.push_back(first_trace);
        }

        Exception()
        {
            time(&creation);
        };

        template <class strT>
        Exception& operator+= (strT txt)
        {
            msg+=txt;
            return *this;
        }

        const char* what() const noexcept
        {
            what_str="";
            what_str += "[";
            what_str += ctime(&creation);
            what_str.resize(what_str.size()-1); // '\n' entfernen
            what_str += "] \"";
            what_str += msg;
            what_str += "\" ";
            for (auto& it: trace)
            {
                what_str+=it;
                if (it!=trace.back())
                    what_str+=", ";
            }
            return what_str.c_str();
        };
    };

    class Logger
    {
    private:
        bool is_active = false;

    public:
        Logger* parent = nullptr;
        std::ostream* dump = &std::cout;
        bool print_time = false;
        std::string prefix;

        Logger()
        { }

        Logger(const std::string& pref) : prefix(pref)
        { }

        template<class msgType>
        void raw_print(msgType msg)
        {
            if (parent==nullptr)
                *dump << msg;
            else
                *parent << msg;
        }

        template<class msgType>
        Logger& operator<<(msgType msg)
        {
            if (!is_active) {
                is_active=true;
                if (print_time && parent==nullptr) {
                    char tmp_c [21];
                    time_t tmp_t = time(nullptr);
                    strftime(tmp_c,21,"%F %T ",localtime(&tmp_t));
                    raw_print(tmp_c);
                }
                raw_print(prefix);
            }
            raw_print(msg);
            return *this;
        }

        Logger& operator<< (std::ostream& (*op)(std::ostream&))
        {
            if (op == &std::endl<std::ostream::char_type,std::ostream::traits_type>)
            {
                is_active = false;
                raw_print(op);
            }
            else
            {
                throw Exception("Der Insertion-Operator wurde unvorhergesehen angewendet","Logger");
            }
            return *this;
        }
    };
}

#endif