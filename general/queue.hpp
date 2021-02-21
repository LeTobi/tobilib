#ifndef TC_QUEUE
#define TC_QUEUE

#include <queue>
#include "exception.hpp"

namespace tobilib
{
    template<class Item>
    class Queue: private std::queue<Item>
    {
    public:
        Item next()
        {
            if (empty())
                throw Exception("Die Queue ist leer","tobilib::Queue::next()");
            Item out = std::queue<Item>::front();
            std::queue<Item>::pop();
            return out;
        }

        void clear()
        {
            while (!std::queue<Item>::empty())
                std::queue<Item>::pop();
        }

        using std::queue<Item>::pop;
        using std::queue<Item>::front;
        using std::queue<Item>::size;
        using std::queue<Item>::empty;
        using std::queue<Item>::push;
    };
}

#endif