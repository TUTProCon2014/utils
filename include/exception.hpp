#pragma once

#include <exception>
#include <stdexcept>
#include <algorithm>
#include <string>



namespace procon { namespace utils {

/** value が false に評価可能であれば例外を投げます。
そうでない場合は、value をそのまま返します。
*/
template <typename T>
auto enforce(T&& value, std::string const & msg, const char* fname = __FILE__, std::size_t line = __LINE__)
    -> decltype(std::forward<T>(value))
{
    if(!value)
        throw std::runtime_error(std::string(fname) + "(" + std::to_string(line) + "): " + msg);

    return std::forward<T>(value);
}


}} // namespace procon::utils