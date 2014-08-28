#pragma once

#include <exception>
#include <stdexcept>
#include <algorithm>
#include <string>



namespace procon { namespace utils {

/** value が false に評価可能であれば例外(`std::runtime_error`)を投げます。
そうでない場合は、value をそのまま返します。

Example:
------------
// 画像が存在しない場合には例外を投げる
auto pb_opt = enforce(Problem::get("img1.ppm"), "cannot open image file.");
auto& pb = *pb_opt;

enforce(pb.div_x() > 0 && pb.div_y() > 0, "invalid problem.");
------------
*/
template <typename T>
auto enforce(T&& value, std::string const & msg, const char* fname = __FILE__, std::size_t line = __LINE__)
    -> decltype(std::forward<T>(value))
{
    if(!value)
        throw std::runtime_error(std::string(fname) + "(" + std::to_string(line) + "): " + msg);

    return std::forward<T>(value);
}


template <typename F>
struct ScopeExit
{
    ScopeExit(F& f) : _f(f) {}
    ScopeExit(ScopeExit&& s) = default; // moveコピーコンストラクタは許可する
    ~ScopeExit(){ _f(); }

  private:
    F& _f;

    ScopeExit(ScopeExit const & s){}
    void operator=(ScopeExit const & s) {}
    void operator=(ScopeExit&& s) {}
};


/**
D言語の`scope(exit)`文です。
そのスコープを抜ける場合に関数を実行します。
使用例としては、例外安全なリソース管理があります。

Example:
------------
int main()
{
    auto obj = new Obj(ctorArgument);

    // たとえ例外が投げられたとしても、確実に`delete`する
    auto scope = scopeExit([&](){
        delete obj;
    });

    maybe_throw_func();

    return 0;
}
------------
*/
template <typename F>
ScopeExit<F> scopeExit(F& f)
{
    return ScopeExit<F>(f);
}

}} // namespace procon::utils