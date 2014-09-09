#pragma once

/**
提供する機能は以下の2つである。
+ 擬似Concept Lite
+ identity


+ 擬似Concept Lite `PROCON_TEMPLATE_CONSTRAINTS`
    C++1z(C++17)で導入が検討されているConcept Liteの擬似機能を提供する。
    Concept Liteとは、軽量Conceptである。
    テンプレートのインスタンス化の際のパラメータに、
    そのパラメータが満たすべき制約条件を設定できる機能である。
    この擬似機能には、`enable_if`を使用したSFINAEを使用している。
    また、validなパラメータか判断するconstexprな関数を構築するためのヘルパとして、
    `PROCON_DEF_TYPE_TRAIT`を提供する。
    <<VC++ Nov 2013 CTPコンパイラは、C++11, C++14コンパイラではない>>ので、
	コンストラクタでも有効なこの方法は使えない。

+ identity
    identityは何もしない関数である。
    ただ受け取った値をそのまま返す。
    identityは、擬似Concept Liteで必要となるconstexpr関数を構築する際に有用である。
    たとえば、ある型が`size_t size();`というようなメンバ関数を持っているかを
    conceptとして知りたいとする。
    擬似Concept Liteでは、次のようにconceptとして使うconstexprな関数を構築する。
    プログラマは、identityが何もしない関数であることを知っているし、
    `identity<size_t>(...)`が`size_t`型を受け取ることもわかる。
    `p->size()`が`size_t`に暗黙変換できない型の値を返す場合は不正となり、
    conceptである関数は、constexprだから、コンパイル時に`false`を返す。

    -------------------------------------------
    PROCON_DEF_TYPE_TRAIT(has_size, true, (
        identity<size_t>(p->size())
    ));

    static assert(has_size<std::array<int, 2>>(), "");    // OK
    static assert(has_size<int>(), "NG");       // NG
    -------------------------------------------
*/

#include <type_traits>
#include "constants.hpp"


#ifdef NOT_SUPPORT_CONSTEXPR
#define PROCON_TEMPLATE_CONSTRAINTS(b) typename void*& = procon::utils::enabler_ptr
#else
#define PROCON_TEMPLATE_CONSTRAINTS(b) typename std::enable_if<(b)>::type *& = procon::utils::enabler_ptr
#endif

#ifdef NOT_SUPPORT_CONSTEXPR
#define PROCON_DEF_TYPE_TRAIT(name, inh, code)
#else
#define PROCON_DEF_TYPE_TRAIT(name, inh, code)  \
template <typename T>                           \
constexpr auto name##_impl(T* p)                \
-> decltype((code, true))                       \
{ return true; }                                \
                                                \
constexpr bool name##_impl(...)                 \
{ return false; }                               \
                                                \
template <typename T>                           \
constexpr bool name()                           \
{                                               \
    return inh && name##_impl(static_cast<      \
    typename std::remove_reference<T>::type*>   \
    (nullptr));                                 \
}
#endif


namespace procon{ namespace utils{

/**
擬似Concept Liteを構築する際に、enable_ifテクニックで使用する
*/
void* enabler_ptr;

template <bool b, typename T>
using Requires = std::enable_if_t<b, T>;

template <typename T>
T identity(T v) { return v; }

}} // namespace procon::utils
