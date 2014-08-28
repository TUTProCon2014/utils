#pragma once

#include <algorithm>
#include <iostream>
#include "exception.hpp"

namespace procon { namespace utils {

/**
http://stackoverflow.com/questions/17671772/c11-variadic-printf-performance

writef(std::cout, "foo % bar % hoge %\n", 1, 2, 3);  みたいに使う
末尾に改行つかない
*/
template <typename Stream>
void writef(Stream & stream, const char *s)
{
    while (*s) {
        if (*s == '%') {
            enforce(*(s + 1) == '%', "invalid format string: missing arguments");
            ++s;
        }
        stream << *s++;
    }
}


template <typename Stream, typename T, typename... Args>
void writef(Stream & stream, const char *s, T&& value, Args&&... args)
{
    while (*s) {
        if (*s == '%') {
            if (*(s + 1) == '%')
                ++s;
            else {
                stream << std::forward<T>(value);
                writef(stream, s + 1, std::forward<Args>(args)...); // call even when *s == 0 to detect extra arguments
                return;
            }
        }
        stream << *s++;
    }
    enforce(0, "extra arguments provided to printf");
}


/**
write(std::cout, 1, ", ", 2, ", ", 3, std::endl); みたいに使う。
末尾に改行つかない
*/
template <typename Stream, typename T>
void write(Stream & stream, T && value)
{
    stream << std::forward<T>(value);
}


template <typename Stream, typename T, typename U, typename... Args>
void write(Stream & stream, T&& value, U&& head, Args&&... args)
{
    stream << std::forward<T>(value);
    write(stream, std::forward<U>(head), std::forward<Args>(args)...);
}


/**
writefの末尾改行ありバージョン
*/
template <typename Stream, typename... Args>
void writefln(Stream & stream, const char *s, Args&&... args)
{
    writef(stream, s, std::forward<Args>(args)...);
    stream << std::endl;
}


/**
writeの末尾改行ありバージョン
*/
template <typename Stream, typename T, typename... Args>
void writeln(Stream & stream, T&& value, Args&&... args)
{
    write(stream, std::forward<T>(value), std::forward<Args>(args)...);
    stream << std::endl;
}

}}