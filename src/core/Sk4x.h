#ifndef Sk4x_DEFINED
#define Sk4x_DEFINED

#include "SkTypes.h"

// First we'll let Clang or GCC try their best with whatever instructions are available.
// Otherwise fall back on portable code.  This really should be a last resort.

#define SK4X_PREAMBLE 1
    #if defined(__clang__)
        #include "Sk4x_clang.h"
    #elif defined(__GNUC__)
        #include "Sk4x_gcc.h"
    #else
        #include "Sk4x_portable.h"
    #endif
#undef SK4X_PREAMBLE

template <typename T> class Sk4x;
typedef Sk4x<int>   Sk4i;
typedef Sk4x<float> Sk4f;

template <typename T> class Sk4x {
public:
    Sk4x();  // Uninitialized; use Sk4x(0,0,0,0) for zero.
    Sk4x(T, T, T, T);
    explicit Sk4x(const T[4]);

    Sk4x(const Sk4x&);
    Sk4x& operator=(const Sk4x&);

    void set(T, T, T, T);

    void store(T[4]) const;

    template <typename Dst> Dst reinterpret() const;
    template <typename Dst> Dst cast() const;

    bool allTrue() const;
    bool anyTrue() const;

    Sk4x bitNot() const;
    Sk4x bitAnd(const Sk4x&) const;
    Sk4x bitOr (const Sk4x&) const;

    Sk4i            equal(const Sk4x&) const;
    Sk4i         notEqual(const Sk4x&) const;
    Sk4i         lessThan(const Sk4x&) const;
    Sk4i      greaterThan(const Sk4x&) const;
    Sk4i    lessThanEqual(const Sk4x&) const;
    Sk4i greaterThanEqual(const Sk4x&) const;

    Sk4x      add(const Sk4x&) const;
    Sk4x subtract(const Sk4x&) const;
    Sk4x multiply(const Sk4x&) const;
    Sk4x   divide(const Sk4x&) const;

    static Sk4x Min(const Sk4x& a, const Sk4x& b);
    static Sk4x Max(const Sk4x& a, const Sk4x& b);

    // Swizzles follow OpenCL xyzw convention.
    Sk4x zwxy() const;

    // When there's a second argument, it's abcd.
    static Sk4x XYAB(const Sk4x& xyzw, const Sk4x& abcd);
    static Sk4x ZWCD(const Sk4x& xyzw, const Sk4x& abcd);

private:
    // It's handy to have Sk4f and Sk4i be mutual friends.
    template <typename S> friend class Sk4x;

#define SK4X_PRIVATE 1
    #if defined(__clang__)
        #include "Sk4x_clang.h"
    #elif defined(__GNUC__)
        #include "Sk4x_gcc.h"
    #else
        #include "Sk4x_portable.h"
    #endif
#undef SK4X_PRIVATE
};

#if defined(__clang__)
    #include "Sk4x_clang.h"
#elif defined(__GNUC__)
    #include "Sk4x_gcc.h"
#else
    #include "Sk4x_portable.h"
#endif

// TODO ideas for enterprising coders:
//   1) Code generated for Max() isn't as good in Sk4x_gcc.h as it is in _clang.  Why?
//   2) Sk4x_sse.h would be good for Windows, and could possibly beat _clang / _gcc
//      (e.g. they can't generate _mm_movemask_ps for allTrue/anyTrue).
//   3) Sk4x_neon.h might be a good idea if _clang / _gcc aren't good enough on ARM.


#endif//Sk4x_DEFINED
