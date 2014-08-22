#pragma once

#include "template.hpp"
#include <cmath>

namespace procon { namespace utils {


PROCON_DEF_TYPE_TRAIT(is_input_range, true,
(
    identity(p->front()),
    identity<bool>(p->empty()),
    p->pop_front()
));


PROCON_DEF_TYPE_TRAIT(is_forward_range, is_input_range<T>(),
(
    identity<T>(p->save())
));


PROCON_DEF_TYPE_TRAIT(is_bidirectional_range, is_forward_range<T>(),
(
    identity<decltype(p->front())>(p->back()),
    p->pop_back()
));


PROCON_DEF_TYPE_TRAIT(is_random_access_range, is_forward_range<T>(),
(
    identity<decltype(p->front())>((*p)[0u])
));


PROCON_DEF_TYPE_TRAIT(is_input_iterator, true,
(
    identity(*(*p)),
    identity<bool>((*p) == (*p)),
    identity<decltype(*p)>(++(*p))
));


/**
*/
template <typename T>
struct Iota
{
    class IotaIterator : public std::iterator<std::forward_iterator_tag, T>
    {
      public:
        IotaIterator & operator++() { ++_a; return *this; }
        IotaIterator & operator--() { --_a; return *this; }
        T operator*() const { return _a; }
        const bool operator!=(IotaIterator const & rhs) const { return _a != rhs._a; }
        const bool operator==(IotaIterator const & rhs) const { return !(*this != rhs); }

        T _a;
    };


    IotaIterator begin() const
    {
        IotaIterator dst;
        dst._a = _a;
        return dst;
    }


    IotaIterator end() const
    {
        IotaIterator dst;
        dst._a = _b;
        return dst;
    }


    T front() const
    {
        return _a;
    }


    void pop_front()
    {
        ++_a;
    }


    bool empty() const
    {
        return _a == _b;
    }


    T _a, _b;
};


/**
*/
template <typename T>
Iota<T> iota(T a, T b)
{
    Iota<T> t;
    t._a = a;
    t._b = b;
    return t;
}

/**
*/
template <typename T>
Iota<T> iota(T a)
{
    return iota<T>(static_cast<T>(0), a);
}


/**
*/
// template <typename T>
// auto vec_total(T const & vec, std::size_t size)
//     -> typename std::remove_const<typename std::remove_reference<decltype(vec[size])>::type>::type
// {
//     decltype(vec_total(vec, size)) sum = 0;

//     for(std::size_t i = 0; i < size; ++i)
//         sum += std::abs(vec[i]);

//     return sum;
// }

}}