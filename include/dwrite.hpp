#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include "exception.hpp"

namespace procon { namespace utils {

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
            enforce(*(s + 1) == '%', "invalid format string: missing arguments");
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
                stream << std::forward<T>(value);
                swritef(stream, s + 1, std::forward<Args>(args)...); // call even when *s == 0 to detect extra arguments
                return;
            }
        }
        stream << *s++;
    }
    enforce(0, "extra arguments provided to printf");
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
    stream << std::forward<T>(value);
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