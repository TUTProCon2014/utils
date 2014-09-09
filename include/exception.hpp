#pragma once

#include <exception>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <tuple>
#include <vector>
#include <type_traits>
#include "backtrace.hpp"


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
#define PROCON_ENFORCE(v, msg) procon::utils::enforce_(v, msg, __FILE__, __LINE__)

template <typename T>
auto enforce_(T&& value, std::string const & msg, const char* fname = __FILE__, std::size_t line = __LINE__)
    -> decltype(std::forward<T>(value))
{
    if(!value){
        std::stringstream info;
        info << fname << "(" << line << "): " << msg << std::endl;

        boost::backtrace bt;
        bt.trace(info);

        throw std::runtime_error(info.str());
    }

    return std::forward<T>(value);
}


template <typename F>
struct ScopeExit
{
    ScopeExit(F& f) : _f(f) {}
    ScopeExit(ScopeExit&& s) = default; // moveコンストラクタは許可する
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



template <typename T, typename Ex>
struct CollectException
{
  public:
    explicit CollectException(std::exception_ptr&& ex) : _ex(ex) {}
    explicit CollectException(T&& rv) : _value(rv) {}
    CollectException(CollectException&& s) = default; // moveコンストラクタは許可する
	CollectException(CollectException& const s) = default; // 断腸の思い, 許すまじVC++

    template <typename F>
    CollectException& onSuccess(F f){
        if(!_ex)
            f(std::forward<T>(_value));

        return *this;
    }


    template <typename F>
    CollectException& onFailure(F f){
        if(_ex){
            try{
                std::rethrow_exception(_ex);
            }
            catch(Ex& ex){
                f(ex);
            }
        }

        return *this;
    }

  private:
    std::exception_ptr _ex;
    T _value;

    //CollectException(CollectException const & s){}
    void operator=(CollectException const & s) {}
    void operator=(CollectException&& s) {}
};


/**
関数を
*/
template <typename Ex, typename F, typename... Args>
auto collectException(F f, Args&&... args) -> CollectException<typename std::remove_reference<decltype(f(std::forward<Args>(args)...))>::type, Ex>
{
    using Ret = decltype(collectException<Ex>(f));

    try{
        return Ret(f(std::forward<Args>(args)...));
    }
    catch(...){
        return Ret(std::current_exception());
    }
}

}} // namespace procon::utils