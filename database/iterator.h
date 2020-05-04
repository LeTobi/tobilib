#ifndef TC_DATABASE_ITERATOR_H
#define TC_DATABASE_ITERATOR_H

#ifndef TC_DATABASE_H
#error nur mit database.h verwenden
#endif

namespace tobilib {
namespace database {

class Iterator
{
public:
    enum class Position {
        rend,
        end,
        inside
    };

    Iterator(const Element&);
    Iterator(); // mandatory
    Iterator(const Iterator&); // mandatory

    Iterator& operator++(); // mandatory
    Iterator operator++(int); // mandatory
    Iterator& operator--();
    Iterator operator--(int);

    bool operator==(const Iterator&) const; // mandatory
    bool operator!=(const Iterator&) const; // mandatory

    Element& operator*(); // mandatory
    Element& operator->(); // mandatory

    Position position();

private:
    Position pos = Position::inside;
    Element instance;
};

} // namespace database
} // namespace tobilib



#endif