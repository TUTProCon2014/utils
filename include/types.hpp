#pragma once

#include <array>

#include "image.hpp"
#include "template.hpp"
#include "dwrite.hpp"

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


template <typename T>
int opCmp(const T& v1, const T& v2);


struct IsInputIteratorCmp {
    template <typename T> static int cmp(const T& v1, const T& v2){
        auto bg1 = v1.begin(), ed1 = v1.end();
        auto bg2 = v2.begin(), ed2 = v2.end();
        while (bg1 != ed1 && bg2 != ed2){
            const int c = opCmp(*bg1, *bg2);
            if(c) return c;

            ++bg1; ++bg2;
        }

        if(bg1 == ed1 && bg2 != ed2) return -1;
        else if(bg1 != ed1 && bg2 == ed2) return 1;
        else return 0;
    }
};


struct IsSimilarToArrayCmp {
    template <typename T> static int cmp(const T& v1, const T& v2){
        auto size_c = opCmp(v1.size(), v2.size());
        if(size_c) return size_c;

        const std::size_t n = v1.size();
        for (std::size_t i = 0; i != n; ++i) {
            const int c = opCmp(v1[i], v2[i]);
            if(c) return c;
        }

        return 0;
    }
};


struct DefaultLessCmp {
    template <typename T> static int cmp(const T& v1, const T& v2){
        if(v1 < v2) return -1;
        else if(v2 < v1) return 1;
        else 0;
    }
};


/**
辞書順比較の比較
*/
template <typename T>
int opCmp(const T& v1, const T& v2)
{
    //std::enable_ifもまともに使えないVC++ 2013 Nov. CTPコンパイラやめてくれ～～～
    // 仕方なく、conditionalで分岐するマン
    return std::conditional_t<is_input_iterator<T>(),
        IsInputIteratorCmp,
        std::conditional_t<is_similar_to_array<T>(),
        IsSimilarToArrayCmp,
        DefaultLessCmp
    >>::cmp(v1, v2);
}

}} // namespace procon::utils