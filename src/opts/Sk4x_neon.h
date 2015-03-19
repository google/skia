// It is important _not_ to put header guards here.
// This file will be intentionally included three times.

#include "SkTypes.h"  // Keep this before any #ifdef for skbug.com/3362

#if defined(SK4X_PREAMBLE)
    #include <arm_neon.h>

    // Template metaprogramming to map scalar types to vector types.
    template <typename T> struct SkScalarToSIMD;
    template <> struct SkScalarToSIMD<float>   { typedef float32x4_t  Type; };
    template <> struct SkScalarToSIMD<int32_t> { typedef int32x4_t Type; };

#elif defined(SK4X_PRIVATE)
    Sk4x(float32x4_t);
    Sk4x(int32x4_t);

    typename SkScalarToSIMD<T>::Type fVec;

#else

// Vector Constructors
//template <> inline Sk4f::Sk4x(int32x4_t v) : fVec(vcvtq_f32_s32(v)) {}
template <> inline Sk4f::Sk4x(float32x4_t v) : fVec(v) {}
template <> inline Sk4i::Sk4x(int32x4_t v) : fVec(v) {}
//template <> inline Sk4i::Sk4x(float32x4_t v) : fVec(vcvtq_s32_f32(v)) {}

// Generic Methods
template <typename T> Sk4x<T>::Sk4x() {}
template <typename T> Sk4x<T>::Sk4x(const Sk4x& other) { *this = other; }
template <typename T> Sk4x<T>& Sk4x<T>::operator=(const Sk4x<T>& other) {
    fVec = other.fVec;
    return *this;
}

// Sk4f Methods
#define M(...) template <> inline __VA_ARGS__ Sk4f::

M() Sk4x(float v) : fVec(vdupq_n_f32(v)) {}
M() Sk4x(float a, float b, float c, float d) {
    // NEON lacks an intrinsic to make this easy.  It is recommended to avoid
    // this constructor unless it is absolutely necessary.

    // I am choosing to use the set lane intrinsics.  Particularly, in the case
    // of floating point, it is likely that the values are already in the right
    // register file, so this may be the best approach.  However, I am not
    // certain that this is the fastest approach and experimentation might be
    // useful.
    fVec = vsetq_lane_f32(a, fVec, 0);
    fVec = vsetq_lane_f32(b, fVec, 1);
    fVec = vsetq_lane_f32(c, fVec, 2);
    fVec = vsetq_lane_f32(d, fVec, 3);
}

// As far as I can tell, it's not possible to provide an alignment hint to
// NEON using intrinsics.  However, I think it is possible at the assembly
// level if we want to get into that.
// TODO: Write our own aligned load and store.
M(Sk4f) Load       (const float fs[4]) { return vld1q_f32(fs); }
M(Sk4f) LoadAligned(const float fs[4]) { return vld1q_f32(fs); }
M(void) store       (float fs[4]) const { vst1q_f32(fs, fVec); }
M(void) storeAligned(float fs[4]) const { vst1q_f32 (fs, fVec); }

template <>
M(Sk4i) reinterpret<Sk4i>() const { return vreinterpretq_s32_f32(fVec); }

template <>
M(Sk4i) cast<Sk4i>() const { return vcvtq_s32_f32(fVec); }

// We're going to skip allTrue(), anyTrue(), and bit-manipulators
// for Sk4f.  Code that calls them probably does so accidentally.
// Ask msarett or mtklein to fill these in if you really need them.
M(Sk4f) add     (const Sk4f& o) const { return vaddq_f32(fVec, o.fVec); }
M(Sk4f) subtract(const Sk4f& o) const { return vsubq_f32(fVec, o.fVec); }
M(Sk4f) multiply(const Sk4f& o) const { return vmulq_f32(fVec, o.fVec); }

M(Sk4f) divide  (const Sk4f& o) const {
    float32x4_t est0 = vrecpeq_f32(o.fVec);
    float32x4_t est1 = vmulq_f32(vrecpsq_f32(est0, o.fVec), est0);
    float32x4_t est2 = vmulq_f32(vrecpsq_f32(est1, o.fVec), est1);
    return vmulq_f32(est2, fVec);
}

M(Sk4f) rsqrt() const {
    float32x4_t est0 = vrsqrteq_f32(fVec);
    float32x4_t est1 = vmulq_f32(vrsqrtsq_f32(fVec, vmulq_f32(est0, est0)), est0);
    float32x4_t est2 = vmulq_f32(vrsqrtsq_f32(fVec, vmulq_f32(est1, est1)), est1);
    return est2;
}

M(Sk4f)  sqrt() const { return this->multiply(this->rsqrt()); }

M(Sk4i) equal           (const Sk4f& o) const { return vreinterpretq_s32_u32(vceqq_f32(fVec, o.fVec)); }
M(Sk4i) notEqual        (const Sk4f& o) const { return vreinterpretq_s32_u32(vmvnq_u32(vceqq_f32(fVec, o.fVec))); }
M(Sk4i) lessThan        (const Sk4f& o) const { return vreinterpretq_s32_u32(vcltq_f32(fVec, o.fVec)); }
M(Sk4i) greaterThan     (const Sk4f& o) const { return vreinterpretq_s32_u32(vcgtq_f32(fVec, o.fVec)); }
M(Sk4i) lessThanEqual   (const Sk4f& o) const { return vreinterpretq_s32_u32(vcleq_f32(fVec, o.fVec)); }
M(Sk4i) greaterThanEqual(const Sk4f& o) const { return vreinterpretq_s32_u32(vcgeq_f32(fVec, o.fVec)); }

M(Sk4f) Min(const Sk4f& a, const Sk4f& b) { return vminq_f32(a.fVec, b.fVec); }
M(Sk4f) Max(const Sk4f& a, const Sk4f& b) { return vmaxq_f32(a.fVec, b.fVec); }

// These shuffle operations are implemented more efficiently with SSE.
// NEON has efficient zip, unzip, and transpose, but it is more costly to
// exploit zip and unzip in order to shuffle.
M(Sk4f) zwxy() const {
    float32x4x2_t zip = vzipq_f32(fVec, vdupq_n_f32(0.0));
    return vuzpq_f32(zip.val[1], zip.val[0]).val[0];
}
// Note that XYAB and ZWCD share code.  If both are needed, they could be
// implemented more efficiently together.  Also, ABXY and CDZW are available
// as well.
M(Sk4f) XYAB(const Sk4f& xyzw, const Sk4f& abcd) {
    float32x4x2_t xayb_zcwd = vzipq_f32(xyzw.fVec, abcd.fVec);
    float32x4x2_t axby_czdw = vzipq_f32(abcd.fVec, xyzw.fVec);
    return vuzpq_f32(xayb_zcwd.val[0], axby_czdw.val[0]).val[0];
}
M(Sk4f) ZWCD(const Sk4f& xyzw, const Sk4f& abcd) {
    float32x4x2_t xayb_zcwd = vzipq_f32(xyzw.fVec, abcd.fVec);
    float32x4x2_t axby_czdw = vzipq_f32(abcd.fVec, xyzw.fVec);
    return vuzpq_f32(xayb_zcwd.val[1], axby_czdw.val[1]).val[0];
}

// Sk4i Methods
#undef M
#define M(...) template <> inline __VA_ARGS__ Sk4i::

M() Sk4x(int32_t v) : fVec(vdupq_n_s32(v)) {}
M() Sk4x(int32_t a, int32_t b, int32_t c, int32_t d) {
    // NEON lacks an intrinsic to make this easy.  It is recommended to avoid
    // this constructor unless it is absolutely necessary.

    // There are a few different implementation strategies.

    // uint64_t ab_i = ((uint32_t) a) | (((uint64_t) b) << 32);
    // uint64_t cd_i = ((uint32_t) c) | (((uint64_t) d) << 32);
    // int32x2_t ab = vcreate_s32(ab_i);
    // int32x2_t cd = vcreate_s32(cd_i);
    // fVec = vcombine_s32(ab, cd);
    // This might not be a bad idea for the integer case.  Either way I think,
    // we will need to move values from general registers to NEON registers.

    // I am choosing to use the set lane intrinsics.  I am not certain that
    // this is the fastest approach.  It may be useful to try the above code
    // for integers.
    fVec = vsetq_lane_s32(a, fVec, 0);
    fVec = vsetq_lane_s32(b, fVec, 1);
    fVec = vsetq_lane_s32(c, fVec, 2);
    fVec = vsetq_lane_s32(d, fVec, 3);
}

// As far as I can tell, it's not possible to provide an alignment hint to
// NEON using intrinsics.  However, I think it is possible at the assembly
// level if we want to get into that.
M(Sk4i) Load       (const int32_t is[4]) { return vld1q_s32(is); }
M(Sk4i) LoadAligned(const int32_t is[4]) { return vld1q_s32(is); }
M(void) store       (int32_t is[4]) const { vst1q_s32(is, fVec); }
M(void) storeAligned(int32_t is[4]) const { vst1q_s32 (is, fVec); }

template <>
M(Sk4f) reinterpret<Sk4f>() const { return vreinterpretq_f32_s32(fVec); }

template <>
M(Sk4f) cast<Sk4f>() const { return vcvtq_f32_s32(fVec); }

M(bool) allTrue() const {
    int32_t a = vgetq_lane_s32(fVec, 0);
    int32_t b = vgetq_lane_s32(fVec, 1);
    int32_t c = vgetq_lane_s32(fVec, 2);
    int32_t d = vgetq_lane_s32(fVec, 3);
    return a & b & c & d;
}
M(bool) anyTrue() const {
    int32_t a = vgetq_lane_s32(fVec, 0);
    int32_t b = vgetq_lane_s32(fVec, 1);
    int32_t c = vgetq_lane_s32(fVec, 2);
    int32_t d = vgetq_lane_s32(fVec, 3);
    return a | b | c | d;
}

M(Sk4i) bitNot() const { return vmvnq_s32(fVec); }
M(Sk4i) bitAnd(const Sk4i& o) const { return vandq_s32(fVec, o.fVec); }
M(Sk4i) bitOr (const Sk4i& o) const { return vorrq_s32(fVec, o.fVec); }

M(Sk4i) equal           (const Sk4i& o) const { return vreinterpretq_s32_u32(vceqq_s32(fVec, o.fVec)); }
M(Sk4i) notEqual        (const Sk4i& o) const { return vreinterpretq_s32_u32(vmvnq_u32(vceqq_s32(fVec, o.fVec))); }
M(Sk4i) lessThan        (const Sk4i& o) const { return vreinterpretq_s32_u32(vcltq_s32(fVec, o.fVec)); }
M(Sk4i) greaterThan     (const Sk4i& o) const { return vreinterpretq_s32_u32(vcgtq_s32(fVec, o.fVec)); }
M(Sk4i) lessThanEqual   (const Sk4i& o) const { return vreinterpretq_s32_u32(vcleq_s32(fVec, o.fVec)); }
M(Sk4i) greaterThanEqual(const Sk4i& o) const { return vreinterpretq_s32_u32(vcgeq_s32(fVec, o.fVec)); }

M(Sk4i) add     (const Sk4i& o) const { return vaddq_s32(fVec, o.fVec); }
M(Sk4i) subtract(const Sk4i& o) const { return vsubq_s32(fVec, o.fVec); }
M(Sk4i) multiply(const Sk4i& o) const { return vmulq_s32(fVec, o.fVec); }
// NEON does not have integer reciprocal, sqrt, or division.
M(Sk4i) Min(const Sk4i& a, const Sk4i& b) { return vminq_s32(a.fVec, b.fVec); }
M(Sk4i) Max(const Sk4i& a, const Sk4i& b) { return vmaxq_s32(a.fVec, b.fVec); }

// These shuffle operations are implemented more efficiently with SSE.
// NEON has efficient zip, unzip, and transpose, but it is more costly to
// exploit zip and unzip in order to shuffle.
M(Sk4i) zwxy() const {
    int32x4x2_t zip = vzipq_s32(fVec, vdupq_n_s32(0.0));
    return vuzpq_s32(zip.val[1], zip.val[0]).val[0];
}
// Note that XYAB and ZWCD share code.  If both are needed, they could be
// implemented more efficiently together.  Also, ABXY and CDZW are available
// as well.
M(Sk4i) XYAB(const Sk4i& xyzw, const Sk4i& abcd) {
    int32x4x2_t xayb_zcwd = vzipq_s32(xyzw.fVec, abcd.fVec);
    int32x4x2_t axby_czdw = vzipq_s32(abcd.fVec, xyzw.fVec);
    return vuzpq_s32(xayb_zcwd.val[0], axby_czdw.val[0]).val[0];
}
M(Sk4i) ZWCD(const Sk4i& xyzw, const Sk4i& abcd) {
    int32x4x2_t xayb_zcwd = vzipq_s32(xyzw.fVec, abcd.fVec);
    int32x4x2_t axby_czdw = vzipq_s32(abcd.fVec, xyzw.fVec);
    return vuzpq_s32(xayb_zcwd.val[1], axby_czdw.val[1]).val[0];
}

#undef M

#endif
