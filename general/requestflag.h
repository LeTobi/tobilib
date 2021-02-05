#ifndef TC_REQUESTFLAG_H
#define TC_REQUESTFLAG_H

#include <set>
#include "queue.hpp"

namespace tobilib
{
    enum class FlagEvent
    {
        requested,
        dismissed
    };

    using FlagRequest = unsigned int;

    class RequestFlag
    {
    public:
        FlagRequest request();
        void dismiss(FlagRequest);

        bool is_requested() const;
        Queue<FlagEvent> events;

    private:
        std::set<FlagRequest> requests;
        unsigned int nextid = 1;
    };
}

#endif