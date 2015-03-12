// It is important _not_ to put header guards here.
// This file will be intentionally included three times.

#include "SkTypes.h"  // Keep this before any #ifdef for skbug.com/3362

#if defined(SK4X_PREAMBLE)
    #include "SkFloatingPoint.h"
    #include <math.h>

#elif defined(SK4X_PRIVATE)
    typedef T Type;
    typedef T Vector[4];

    Vector fVec;

    template <int m, int a, int s, int k>
    static Sk4x Shuffle(const Sk4x&, const Sk4x&);

    void set(const T vals[4]) { for (int i = 0; i < 4; i++) { fVec[i] = vals[i]; } }

#else

#define M(...) template <typename T> __VA_ARGS__ Sk4x<T>::

M() Sk4x() {}
M() Sk4x(T v) { fVec[0] = fVec[1] = fVec[2] = fVec[3] = v; }
M() Sk4x(T a, T b, T c, T d) { fVec[0] = a; fVec[1] = b; fVec[2] = c; fVec[3] = d; }

M()              Sk4x(const Sk4x<T>& other) { this->set(other.fVec);               }
M(Sk4x<T>&) operator=(const Sk4x<T>& other) { this->set(other.fVec); return *this; }

M(Sk4x<T>) Load       (const T vals[4]) { Sk4x r; r.set(vals); return r; }
M(Sk4x<T>) LoadAligned(const T vals[4]) { return Load(vals);             }

M(void) store       (T vals[4]) const { for (int i = 0; i < 4; i++) { vals[i] = fVec[i]; } }
M(void) storeAligned(T vals[4]) const { this->store(vals); }

M(template <typename Dst> Dst) reinterpret() const {
    Dst d;
    memcpy(&d.fVec, fVec, sizeof(fVec));
    return d;
}
M(template <typename Dst> Dst) cast() const {
    return Dst((typename Dst::Type)fVec[0],
               (typename Dst::Type)fVec[1],
               (typename Dst::Type)fVec[2],
               (typename Dst::Type)fVec[3]);
}

M(bool) allTrue() const { return fVec[0] && fVec[1] && fVec[2] && fVec[3]; }
M(bool) anyTrue() const { return fVec[0] || fVec[1] || fVec[2] || fVec[3]; }

M(Sk4x<T>) bitNot() const { return Sk4x(~fVec[0], ~fVec[1], ~fVec[2], ~fVec[3]); }

#define BINOP(op) fVec[0] op other.fVec[0], \
                  fVec[1] op other.fVec[1], \
                  fVec[2] op other.fVec[2], \
                  fVec[3] op other.fVec[3]
M(Sk4x<T>)   bitAnd(const Sk4x& other)    const { return Sk4x(BINOP(&)); }
M(Sk4x<T>)    bitOr(const Sk4x& other)    const { return Sk4x(BINOP(|)); }
M(Sk4x<T>)      add(const Sk4x<T>& other) const { return Sk4x(BINOP(+)); }
M(Sk4x<T>) subtract(const Sk4x<T>& other) const { return Sk4x(BINOP(-)); }
M(Sk4x<T>) multiply(const Sk4x<T>& other) const { return Sk4x(BINOP(*)); }
M(Sk4x<T>)   divide(const Sk4x<T>& other) const { return Sk4x(BINOP(/)); }
#undef BINOP

template<> inline Sk4f Sk4f::rsqrt() const {
    return Sk4f(sk_float_rsqrt(fVec[0]),
                sk_float_rsqrt(fVec[1]),
                sk_float_rsqrt(fVec[2]),
                sk_float_rsqrt(fVec[3]));
}

template<> inline Sk4f Sk4f::sqrt() const {
    return Sk4f(sqrtf(fVec[0]),
                sqrtf(fVec[1]),
                sqrtf(fVec[2]),
                sqrtf(fVec[3]));
}

#define BOOL_BINOP(op) fVec[0] op other.fVec[0] ? -1 : 0, \
                       fVec[1] op other.fVec[1] ? -1 : 0, \
                       fVec[2] op other.fVec[2] ? -1 : 0, \
                       fVec[3] op other.fVec[3] ? -1 : 0
M(Sk4i)            equal(const Sk4x<T>& other) const { return Sk4i(BOOL_BINOP(==)); }
M(Sk4i)         notEqual(const Sk4x<T>& other) const { return Sk4i(BOOL_BINOP(!=)); }
M(Sk4i)         lessThan(const Sk4x<T>& other) const { return Sk4i(BOOL_BINOP( <)); }
M(Sk4i)      greaterThan(const Sk4x<T>& other) const { return Sk4i(BOOL_BINOP( >)); }
M(Sk4i)    lessThanEqual(const Sk4x<T>& other) const { return Sk4i(BOOL_BINOP(<=)); }
M(Sk4i) greaterThanEqual(const Sk4x<T>& other) const { return Sk4i(BOOL_BINOP(>=)); }
#undef BOOL_BINOP

M(Sk4x<T>) Min(const Sk4x<T>& a, const Sk4x<T>& b) {
    return Sk4x(SkTMin(a.fVec[0], b.fVec[0]),
                SkTMin(a.fVec[1], b.fVec[1]),
                SkTMin(a.fVec[2], b.fVec[2]),
                SkTMin(a.fVec[3], b.fVec[3]));
}

M(Sk4x<T>) Max(const Sk4x<T>& a, const Sk4x<T>& b) {
    return Sk4x(SkTMax(a.fVec[0], b.fVec[0]),
                SkTMax(a.fVec[1], b.fVec[1]),
                SkTMax(a.fVec[2], b.fVec[2]),
                SkTMax(a.fVec[3], b.fVec[3]));
}

M(template <int m, int a, int s, int k> Sk4x<T>) Shuffle(const Sk4x<T>& x, const Sk4x<T>& y) {
    return Sk4x(m < 4 ? x.fVec[m] : y.fVec[m-4],
                a < 4 ? x.fVec[a] : y.fVec[a-4],
                s < 4 ? x.fVec[s] : y.fVec[s-4],
                k < 4 ? x.fVec[k] : y.fVec[k-4]);
}

M(Sk4x<T>) zwxy() const                             { return Shuffle<2,3,0,1>(*this, *this); }
M(Sk4x<T>) XYAB(const Sk4x& xyzw, const Sk4x& abcd) { return Shuffle<0,1,4,5>( xyzw,  abcd); }
M(Sk4x<T>) ZWCD(const Sk4x& xyzw, const Sk4x& abcd) { return Shuffle<2,3,6,7>( xyzw,  abcd); }

#undef M

#endif
