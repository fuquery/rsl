#pragma once

#define RSL_OS_UNKNOWN 0
#define RSL_OS_WINDOWS 1
#define RSL_OS_LINUX   2
#define RSL_OS_MAC     3
#define RSL_OS_FREEBSD 4
#define RSL_OS_ANDROID 5
#define RSL_OS_IOS     6

#if defined(_WIN32) || defined(_WIN64)
    #define RSL_OS RSL_OS_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        #define RSL_OS RSL_OS_IOS
    #else
        #define RSL_OS RSL_OS_MAC
    #endif
#elif defined(__ANDROID__)
    #define RSL_OS RSL_OS_ANDROID
#elif defined(__FreeBSD__)
    #define RSL_OS RSL_OS_FREEBSD
#elif defined(__linux__)
    #define RSL_OS RSL_OS_LINUX
#else
    #define RSL_OS RSL_OS_UNKNOWN
#endif