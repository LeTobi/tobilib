#include "html.h"

namespace tobilib::html
{
    StringPlus toTextContent(const StringPlus& raw)
    {
        return raw.replace_all("&","&amp;").replace_all("<","&lt;").replace_all(">","&gt;");
    }
}