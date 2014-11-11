#ifndef Sk4x_DEFINED
#define Sk4x_DEFINED

#include "SkTypes.h"

#define SK4X_PREAMBLE 1
    #include "Sk4x_portable.h"
#undef SK4X_PREAMBLE

template <typename T> class Sk4x;
typedef Sk4x<float>   Sk4f;
typedef Sk4x<int32_t> Sk4i;

template <typename T> class Sk4x {
public:
    Sk4x();  // Uninitialized; use Sk4x(0,0,0,0) for zero.
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
    Sk4x      add(const Sk4x&) const;
    Sk4x subtract(const Sk4x&) const;
    Sk4x multiply(const Sk4x&) const;
    Sk4x   divide(const Sk4x&) const;

    Sk4i            equal(const Sk4x&) const;
    Sk4i         notEqual(const Sk4x&) const;
    Sk4i         lessThan(const Sk4x&) const;
    Sk4i      greaterThan(const Sk4x&) const;
    Sk4i    lessThanEqual(const Sk4x&) const;
    Sk4i greaterThanEqual(const Sk4x&) const;

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
    #include "Sk4x_portable.h"
#undef SK4X_PRIVATE
};

#include "Sk4x_portable.h"

#endif//Sk4x_DEFINED
