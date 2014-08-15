#pragma once

#include <type_traits>


#define PROCON_TEMPLATE_CONSTRAINTS(b) typename std::enable_if<(b)>::type *& = procon::utils::enabler_ptr


#define PROCON_DEF_TYPE_TRAIT(name, inh, code)   \
template <typename T>                       \
constexpr auto name##_impl(T* p)            \
-> decltype((code, true))              \
{ return true; }                            \
                                            \
constexpr bool name##_impl(...)             \
{ return false; }                           \
                                            \
template <typename T>                       \
constexpr bool name()                       \
{                                           \
    return inh && name##_impl(static_cast<         \
    typename std::remove_reference<T>::type*>   \
    (nullptr));                                 \
}


#define PROCON_DEF_STRUCT_FUNCTION(name, func)  \
struct name {                                   \
    template <typename... A>                    \
    auto operator()(A&&... args)                \
    { return func(std::forward<A>(args)...); }  \
};

namespace procon{ namespace utils{

extern void* enabler_ptr;

template <typename T>
auto identity(T v) { return v; }

}} // namespace procon::utils