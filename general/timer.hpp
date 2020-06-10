#ifndef TC_TIMER_H
#define TC_TIMER_H

#include <chrono>

namespace tobilib
{
    class Timer
    {
    private:
        int limit = 0;
        std::chrono::system_clock::time_point start;
        bool _enabled = false;

    public:
        Timer& set() {
            start = std::chrono::system_clock::now();
            _enabled = true;
            return *this;
        };

        /** aktiviert den Timer
         * @param t Zeit in sekunden
         */
        Timer& set(double t) {
            limit=t*1000;
            return set();
        };

        bool is_enabled() const {
            return _enabled;
        }

        void disable() {
            _enabled = false;
        };

        double left() const {
            if (!_enabled) return 0;
            return (double)(limit - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-start).count()) / 1000;
        }

        bool due() const {
            if (!_enabled) return false;
            return left()<=0;
        };

        Timer() {};
        Timer(double _limit): limit(1000*_limit) {};
    };
}

#endif