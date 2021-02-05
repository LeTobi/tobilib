#include "requestflag.h"
#include "exception.hpp"

using namespace tobilib;

FlagRequest RequestFlag::request()
{
    if (requests.empty())
        events.push(FlagEvent::requested);
    requests.insert(nextid);
    return nextid++;
}

void RequestFlag::dismiss(FlagRequest id)
{
    if (requests.count(id) == 0)
        throw Exception("Symmetrie von request/dismiss ist fehlgeschlagen","tobilib::RequestFlag::dismiss()");
    requests.erase(id);
    if (requests.empty())
        events.push(FlagEvent::dismissed);
}

bool RequestFlag::is_requested() const
{
    return !requests.empty();
}