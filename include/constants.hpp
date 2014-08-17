#pragma once

namespace procon{ namespace utils {

enum class Target
{
    Windows, OSX, Linux,
};


#ifdef _MSC_VER > 0
    #define TARGET_WINDOWS
    #define NOT_SUPPORT_CONSTEXPR
    const Target buildTarget = Target::Windows;
#elif defined( __APPLE_CC__)
    #define TARGET_OSX
    constexpr Target buildTarget = Target::OSX;
#else
    #define TARGET_LINUX
    constexpr Target buildTarget = Target::Linux;
#endif


}}  // namespace procon::utils
