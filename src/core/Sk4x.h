#ifndef Sk4x_DEFINED
#define Sk4x_DEFINED

#include "SkTypes.h"

#define SK4X_PREAMBLE 1
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
        #include "../opts/Sk4x_sse.h"
    #elif defined(SK_ARM_HAS_NEON)
        #include "../opts/Sk4x_neon.h"
    #else
        #include "../opts/Sk4x_none.h"
    #endif
#undef SK4X_PREAMBLE

template <typename T> class Sk4x;
typedef Sk4x<float>   Sk4f;
typedef Sk4x<int32_t> Sk4i;

// Some Sk4x methods are implemented only for Sk4f or Sk4i.
// They might be unavailable, really slow, or just a bad idea.
// Talk to mtklein if you find yourself unable to link and
// really need one of those methods.

template <typename T> class Sk4x {
public:
    Sk4x();  // Uninitialized; use Sk4x(0) for zero.
    explicit Sk4x(T);  // Same as Sk4x(T,T,T,T);
    Sk4x(T, T, T, T);

    Sk4x(const Sk4x&);
    Sk4x& operator=(const Sk4x&);

    static Sk4x Load       (const T[4]);
    static Sk4x LoadAligned(const T[4]);

    void store       (T[4]) const;
    void storeAligned(T[4]) const;

    template <typename Dst> Dst reinterpret() const;
    template <typename Dst> Dst        cast() const;

    bool allTrue() const;
    bool anyTrue() const;

    Sk4x   bitNot()            const;
    Sk4x   bitAnd(const Sk4x&) const;
    Sk4x    bitOr(const Sk4x&) const;
    // TODO: Sk4x bitAndNot(const Sk4x&) const; is efficient in SSE.
    Sk4x      add(const Sk4x&) const;
    Sk4x subtract(const Sk4x&) const;
    Sk4x multiply(const Sk4x&) const;
    Sk4x   divide(const Sk4x&) const;

    // TODO: why doesn't MSVC like operator~() ?
    //Sk4x operator ~()              const { return this->bitNot(); }
    Sk4x operator &(const Sk4x& o) const { return this->bitAnd(o); }
    Sk4x operator |(const Sk4x& o) const { return this->bitOr (o); }
    Sk4x operator +(const Sk4x& o) const { return this->add(o); }
    Sk4x operator -(const Sk4x& o) const { return this->subtract(o); }
    Sk4x operator *(const Sk4x& o) const { return this->multiply(o); }
    Sk4x operator /(const Sk4x& o) const { return this->divide(o); }

    Sk4x& operator &=(const Sk4x& o) { return (*this = *this & o); }
    Sk4x& operator |=(const Sk4x& o) { return (*this = *this | o); }
    Sk4x& operator +=(const Sk4x& o) { return (*this = *this + o); }
    Sk4x& operator -=(const Sk4x& o) { return (*this = *this - o); }
    Sk4x& operator *=(const Sk4x& o) { return (*this = *this * o); }
    Sk4x& operator /=(const Sk4x& o) { return (*this = *this / o); }

    Sk4x rsqrt() const;   // Approximate reciprocal sqrt().
    Sk4x  sqrt() const;   // this->multiply(this->rsqrt()) may be faster, but less precise.

    Sk4i            equal(const Sk4x&) const;
    Sk4i         notEqual(const Sk4x&) const;
    Sk4i         lessThan(const Sk4x&) const;
    Sk4i      greaterThan(const Sk4x&) const;
    Sk4i    lessThanEqual(const Sk4x&) const;
    Sk4i greaterThanEqual(const Sk4x&) const;

    Sk4i operator ==(const Sk4x& o) const { return this->equal(o); }
    Sk4i operator !=(const Sk4x& o) const { return this->notEqual(o); }
    Sk4i operator  <(const Sk4x& o) const { return this->lessThan(o); }
    Sk4i operator  >(const Sk4x& o) const { return this->greaterThan(o); }
    Sk4i operator <=(const Sk4x& o) const { return this->lessThanEqual(o); }
    Sk4i operator >=(const Sk4x& o) const { return this->greaterThanEqual(o); }

    static Sk4x Min(const Sk4x& a, const Sk4x& b);
    static Sk4x Max(const Sk4x& a, const Sk4x& b);

    // Swizzles follow OpenCL xyzw convention.
    Sk4x zwxy() const;

    // When there's a second argument, it's abcd.
    static Sk4x XYAB(const Sk4x& xyzw, const Sk4x& abcd);
    static Sk4x ZWCD(const Sk4x& xyzw, const Sk4x& abcd);

    // TODO: these are particularly efficient in SSE.  Useful?  Also efficient in NEON?
    // static Sk4x XAYB(const Sk4x& xyzw, const Sk4x& abcd);
    // static Sk4x ZCWD(const Sk4x& xyzw, const Sk4x& abcd);

private:
    // It's handy to have Sk4f and Sk4i be mutual friends.
    template <typename S> friend class Sk4x;

#define SK4X_PRIVATE 1
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
        #include "../opts/Sk4x_sse.h"
    #elif defined(SK_ARM_HAS_NEON)
        #include "../opts/Sk4x_neon.h"
    #else
        #include "../opts/Sk4x_none.h"
    #endif
#undef SK4X_PRIVATE
};

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include "../opts/Sk4x_sse.h"
#elif defined(SK_ARM_HAS_NEON)
    #include "../opts/Sk4x_neon.h"
#else
    #include "../opts/Sk4x_none.h"
#endif

#endif//Sk4x_DEFINED
