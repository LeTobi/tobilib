#ifndef TC_TIMER_H
#define TC_TIMER_H

#include <ctime>

namespace tobilib
{
    class Timer
    {
    private:
        double limit = 0;
        time_t start = 0;

    public:
        Timer& set() {
            time(&start);
            return *this;
        };

        Timer& set(double t) {
            limit=t;
            return set();
        };

        void disable() {
            start = 0;
        };

        double left() const {
            if (start==0) return 0;
            time_t now;
            time(&now);
            return limit - difftime(now,start);
        }

        bool due() const {
            if (start==0) return false;
            return left()<=0;
        };

        Timer() {};
        Timer(double _limit): limit(_limit) {};
    };
}

#endif