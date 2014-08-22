#pragma once

namespace procon{ namespace utils {

enum class Target
{
    Windows, OSX, Linux,
};


#ifdef _MSC_VER
    #define TARGET_WINDOWS
    // #define NOT_SUPPORT_CONSTEXPR
    #define NOT_SUPPORT_TEMPLATE_CONSTRAINTS
    const Target buildTarget = Target::Windows;
#elif defined(__APPLE_CC__)
    #define TARGET_OSX
    constexpr Target buildTarget = Target::OSX;
#else
    #define TARGET_LINUX
    constexpr Target buildTarget = Target::Linux;
#endif


#ifndef NOT_SUPPORT_CONSTEXPR
    #define SUPPORT_CONSTEXPR
#endif

#ifndef NOT_SUPPORT_TEMPLATE_CONSTRAINTS
    #define SUPPORT_TEMPLATE_CONSTRAINTS
#endif

}}  // namespace procon::utils
