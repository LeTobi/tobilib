#ifndef TC_ENCODING_HTML_H
#define TC_ENCODING_HTML_H

#include "../stringplus/stringplus.h"

namespace tobilib{ namespace html
{
    StringPlus toTextContent(const StringPlus&);
}}

#ifdef TC_AS_HPP
    #include "html.cpp"
#endif

#endif