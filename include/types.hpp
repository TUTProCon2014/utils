#pragma once

#include <array>

#include "image.hpp"
#include "template.hpp"

namespace procon { namespace utils {

/**
4方向を表す列挙型
*/
enum class Direction
{
    right,
    up,
    left,
    down,
};


/** 2次元インデックス
*/
typedef std::array<std::size_t, 2> Index2D;


/// ditto
Index2D makeIndex2D(std::size_t i, std::size_t j)
{
    Index2D idx;
    idx[0] = i;
    idx[1] = j;
    return idx;
}


///
template <typename T>
int opCmp(T a, T b)
{
    if(a < b) return -1;
    else if (a == b) return 0;
    else return 1;
}

}} // namespace procon::utils