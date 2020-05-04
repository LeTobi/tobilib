#include "database.h"

using namespace tobilib;
using namespace database;

Iterator::Iterator(const Element& element): instance(element)
{ }

Iterator::Iterator()
{ }

Iterator::Iterator(const Iterator& other)
{ 
    if (other==*this)
        return;
    instance = other.instance;
    pos = other.pos;
}

Iterator& Iterator::operator++()
{

}

Iterator Iterator::operator++(int)
{
    Iterator copy = *this;
    ++*this;
    return copy;
}

Iterator& Iterator::operator--()
{

}

Iterator Iterator::operator--(int)
{
    Iterator copy = *this;
    --*this;
    return copy;
}

bool Iterator::operator==(const Iterator& other) const
{
    return pos==other.pos && instance==other.instance;
}

bool Iterator::operator!=(const Iterator& other) const
{
    return !(*this==other);
}

Element& Iterator::operator*()
{
    if (pos!=Position::inside)
        return Element::null;
    return instance;
}

Element& Iterator::operator->()
{
    return *(*this);
}

Iterator::Position Iterator::position()
{
    return pos;
}