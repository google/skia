#ifndef SkTypes_DEFINED
#define SkTypes_DEFINED

#include <stdint.h>
#include <type_traits>

#include "SkPreConfig.h"
#include "SkPostConfig.h"

#ifndef SkDebugf
    SK_API void SkDebugf(const char format[], ...);
#endif

#define SK_INIT_TO_AVOID_WARNING    = 0

    #define SkASSERT(cond)            static_cast<void>(0)
    #define SkDEBUGFAIL(message)
    #define SkDEBUGCODE(...)
    #define SkAssertResult(cond)         if (cond) {} do {} while(false)

#define SkToBool(cond)  ((cond) != 0)

template <typename D, typename S> constexpr D SkTo(S s) {
    return static_cast<D>(s);
}
#define SkToU8(x)    SkTo<uint8_t>(x)

#define SK_MinS32   -SK_MaxS32
#define SK_MaxS32   0x7FFFFFFF
#define SK_NaN32    ((int) (1U << 31))

template <typename T, size_t N> char (&SkArrayCountHelper(T (&array)[N]))[N];
#define SK_ARRAY_COUNT(array) (sizeof(SkArrayCountHelper(array)))

template <typename T> static inline T SkTAbs(T value) {
    if (value < 0) {
        value = -value;
    }
    return value;
}

template <typename T> constexpr const T& SkTMin(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template <typename T> constexpr const T& SkTMax(const T& a, const T& b) {
    return (b < a) ? a : b;
}

template <typename T> static constexpr const T& SkTPin(const T& value, const T& min, const T& max) {
    return SkTMax(SkTMin(value, max), min);
}

template <typename T> static inline void SkTSwap(T& a, T& b) {
    T c(std::move(a));
    a = std::move(b);
    b = std::move(c);
}

template <typename Dst> Dst SkTCast(const void* ptr) {
    union {
        const void* src;
        Dst dst;
    } data;
    data.src = ptr;
    return data.dst;
}

#endif
