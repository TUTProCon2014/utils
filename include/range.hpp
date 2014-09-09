#pragma once

#include "template.hpp"
#include <cmath>
#include <algorithm>
#include <type_traits>
#include <boost/range/irange.hpp>
#include "exception.hpp"
#include "dwrite.hpp"


namespace procon { namespace utils {

// 互換性を残すために、boost::irangeのラッパとする
/** [a, b)の範囲の数列を生成します
*/
auto iota(size_t a, size_t b, ptrdiff_t step)
-> decltype(boost::irange(a, b, step))
{
    PROCON_ENFORCE(step != 0, "step is 0");
    PROCON_ENFORCE(step > 0 ? a <= b : a >= b, utils::format("invalid range [%, %), step: %", a, b, step));

    return boost::irange(a, b, step);
}


/// ditto
auto iota(size_t a, size_t b)
-> decltype(boost::irange(a, b))
{
    PROCON_ENFORCE(a <= b, utils::format("invalid range [%, %)", a, b));

    return boost::irange(a, b);
}


/// [0, 1, 2, ..., a-1] という数列を生成します
auto iota(size_t a)
-> decltype(iota(0, a))
{
    return iota(0, a);
}


/**
std::equalのヘルパー関数
イテレータじゃなくて、サイズも考慮に入れる
*/
template <typename T, typename U>
bool equal(T const & t, U const & u)
{
    if(t.size() != u.size())
        return false;

    return std::equal(t.begin(), t.end(), u.begin());
}


}}