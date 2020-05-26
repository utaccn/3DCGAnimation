#pragma once
// External libraries are usually not perfect (especially older ones) so they may generate
// many compile warnings when we use a high warning level on our own code. With these macros
// we disable compiler warnings between the DISABLE_WARNINGS_PUSH() and DISABLE_WARNINGS_POP
// functions.

#if defined(__clang__)
#define CLANG 1
#elif defined(__GNUC__)
#define GCC 1
#elif defined(_MSC_VER)
#define MSVC 1
#endif

#if defined(CLANG)
#define DISABLE_WARNINGS_PUSH() _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wunused-function\"")
#define DISABLE_WARNINGS_POP() _Pragma("clang diagnostic pop")
#elif defined(GCC)
#define DISABLE_WARNINGS_PUSH()
#define DISABLE_WARNINGS_POP()
#elif defined(MSVC)
#define DISABLE_WARNINGS_PUSH() __pragma(warning(push, 0)) __pragma(warning())
#define DISABLE_WARNINGS_POP() __pragma(warning(pop))
#else
#define DISABLE_WARNINGS_PUSH()
#define DISABLE_WARNINGS_POP()
#endif