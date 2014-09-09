#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <type_traits>
#include "exception.hpp"
#include "range.hpp"

/*
統一的で、簡単な出力及び文字列変換を提供します。
iostreamではめんどくさい、という人向けの機能です。
デバッグにご活用ください。

Example:
---------------
// 型安全なprint, 不正な引数はコンパイル時にエラーとなる
writeln("Hello, World!", "--", 1, true);

// 型安全なprintf, 不正な引数はコンパイル時にエラーとなる。
// フォーマット`%`に対して、引数の数が多すぎたりする場合は実行時例外
writefln("format % -- %", std::vector<bool>(4, true), makeIndex2D(1, 2));

// std::stringstreamなど、任意のストリームに出力したい場合には、先頭に`s`をつける
std::stringstream ss;
swriteln(ss, "format-output % -- %", 1, 2);

// フォーマット出力された文字列を得たい場合には、`format`を使う
std::string str = format("fooo(%, %)", makeIndex2D(1, 2), std::vector<bool>(2, true));

// フォーマットなし、ただ単に結合した文字列が欲しい場合には`text`を使う
std::string str = text(1, " : ", makeIndex2D(2, 2));

// ユーザー定義型については、次の順序でコンパイル時に確認される.
// どのパターンにもマッチしない場合、コンパイル時エラーとなる
//      + value.to_string(std::ostream&)                オーバーロード
//      + std::cout << value                            標準フォーマット
//      + イテレータである                              [%, %, %, ...]みたいに展開
//      + value[value.size()-1] のコンパイルが通る       同上
//      + std::cout << value.what() のコンパイルが通る   std::exception用
---------------
*/

namespace procon { namespace utils {


PROCON_DEF_TYPE_TRAIT(has_to_string, true,
(
    p->to_string(std::declval<std::ostream&>())
));

PROCON_DEF_TYPE_TRAIT(can_stream_out, true,
(
    std::declval<std::ostream&>() << *p
));

PROCON_DEF_TYPE_TRAIT(is_input_iterator, true,
(
    identity(*(*p)),
    identity<bool>((*p) == (*p)),
    identity<decltype(*p)>(++(*p))
));

PROCON_DEF_TYPE_TRAIT(is_similar_to_array, true,
(
    identity<std::size_t>(p->size()),
    identity((*p)[std::declval<std::size_t>()])
));

PROCON_DEF_TYPE_TRAIT(is_similar_to_exception, true,
(
    std::declval<std::ostream&>() << p->what()
));



template <typename Stream, typename T>
void swriteOne(Stream& stream, T&& value);


struct HasToStringWriter { template <typename Stream, typename T> static void writer(Stream& s, T&& value){ value.to_string(s); } };
struct CanStreamOutWriter { template <typename Stream, typename T> static void writer(Stream& s, T&& value){ s << std::forward<T>(value); } };
struct IsInputIteratorWriter {
    template <typename Stream, typename T> static void writer(Stream& s, T&& value){
        auto bg = value.begin(), ed = value.end(); bool b = false; s << "[";
        while (!(bg == ed)){ if (b) s << ", "; b = true; swriteOne(s, *bg); ++bg; } s << "]";
    }
};
struct IsSimilarToArrayWriter {
    template <typename Stream, typename T> static void writer(Stream& s, T&& value){
        std::size_t n = value.size(); bool b = false; s << "[";
        for (std::size_t i = 0; i != n; ++i) { if (b) s << ", "; b = true; swriteOne(s, value[i]); } s << "]";
    }
};
struct IsSimilarToExceptionWriter { template <typename Stream, typename T> static void writer(Stream& s, T&& value){ s << value.what(); } };
struct StaticAssertWriter { template <typename... Args> static void writer(Args&&...){
    // sizeof...(Args) >= 0の部分は常にfalseになる
    // つまり、この関数がインスタンス化された時点でコンパイル時エラー
    static_assert(sizeof...(Args) >= 0, "Overload mismatch"); }
};


template <typename Stream, typename T>
void swriteOne(Stream& stream, T&& value)
{
    //std::enable_ifもまともに使えないVC++ 2013 Nov. CTPコンパイラやめてくれ～～～
    // 仕方なく、conditionalで分岐するマン
    std::conditional_t<has_to_string<T>(),
        HasToStringWriter,
        std::conditional_t<can_stream_out<T>(),
        CanStreamOutWriter,
        std::conditional_t<is_input_iterator<T>(),
        IsInputIteratorWriter,
        std::conditional_t<is_similar_to_array<T>(),
        IsSimilarToArrayWriter,
        std::conditional_t<is_similar_to_exception<T>(),
        IsSimilarToExceptionWriter,
        StaticAssertWriter
    >>>>>::writer(stream, value);
}



/**
http://stackoverflow.com/questions/17671772/c11-variadic-printf-performance

swritef(std::cout, "foo % bar % hoge %\n", 1, 2, 3);  みたいに使う
末尾に改行つかない
*/
template <typename Stream>
void swritef(Stream & stream, const char *s)
{
    while (*s) {
        if (*s == '%') {
            PROCON_ENFORCE(*(s + 1) == '%', "invalid format string: missing arguments");
            ++s;
        }
        stream << *s++;
    }
}


template <typename Stream, typename T, typename... Args>
void swritef(Stream & stream, const char *s, T&& value, Args&&... args)
{
    while (*s) {
        if (*s == '%') {
            if (*(s + 1) == '%')
                ++s;
            else {
                swriteOne(stream, std::forward<T>(value));
                swritef(stream, s + 1, std::forward<Args>(args)...); // call even when *s == 0 to detect extra arguments
                return;
            }
        }
        stream << *s++;
    }
    PROCON_ENFORCE(0, "extra arguments provided to printf");
}


/**
swrite(std::cout, 1, ", ", 2, ", ", 3, std::endl); みたいに使う。
末尾に改行つかない
*/
template <typename Stream>
void swrite(Stream & stream) {}


template <typename Stream, typename T, typename... Args>
void swrite(Stream & stream, T&& value, Args&&... args)
{
    swriteOne(stream, std::forward<T>(value));
    swrite(stream, std::forward<Args>(args)...);
}


/**
swritefの末尾改行ありバージョン
*/
template <typename Stream, typename... Args>
void swritefln(Stream & stream, const char *s, Args&&... args)
{
    swritef(stream, s, std::forward<Args>(args)...);
    stream << std::endl;
}


/**
swriteの末尾改行ありバージョン
*/
template <typename Stream, typename... Args>
void swriteln(Stream & stream, Args&&... args)
{
    swrite(stream, std::forward<Args>(args)...);
    stream << std::endl;
}


template <typename... Args>
void writef(const char *s, Args&&... args)
{
    swritef(std::cout, s, std::forward<Args>(args)...);
}


template <typename... Args>
void writefln(const char *s, Args&&... args)
{
    swritefln(std::cout, s, std::forward<Args>(args)...);
}


template <typename... Args>
void write(Args&&... args)
{
    swrite(std::cout, std::forward<Args>(args)...);
}


template <typename... Args>
void writeln(Args&&... args)
{
    swriteln(std::cout, std::forward<Args>(args)...);
}


template <typename... Args>
std::string format(const char *s, Args&&... args)
{
    std::stringstream ss;
    swritef(ss, s, std::forward<Args>(args)...);
    return ss.str();
}


template <typename... Args>
std::string text(Args&&... args)
{
    std::stringstream ss;
    swrite(ss, std::forward<Args>(args)...);
    return ss.str();
}
}}
