#pragma once

#include <array>

#include "image.hpp"

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
using Index2D = std::array<std::size_t, 2>;

}} // namespace procon::utils