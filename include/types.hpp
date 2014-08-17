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
#ifdef TARGET_WINDOWS
#define Index2D std::array<std::size_t, 2>
#elif
using Index2D = std::array<std::size_t, 2>;
#endif

Index2D makeIndex2D(std::size_t i, std::size_t j)
{
    Index2D idx;
    idx[0] = i;
    idx[1] = j;
    return idx;
}

}} // namespace procon::utils