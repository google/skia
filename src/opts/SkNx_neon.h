/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_neon_DEFINED
#define SkNx_neon_DEFINED

#include <arm_neon.h>

template <>
class SkNi<2, int32_t> {
public:
    SkNi(int32x2_t vec) : fVec(vec) {}

    SkNi() {}
    bool allTrue() const { return vget_lane_s32(fVec, 0) && vget_lane_s32(fVec, 1); }
    bool anyTrue() const { return vget_lane_s32(fVec, 0) || vget_lane_s32(fVec, 1); }
private:
    int32x2_t fVec;
};

template <>
class SkNi<4, int32_t> {
public:
    SkNi(int32x4_t vec) : fVec(vec) {}

    SkNi() {}
    bool allTrue() const { return vgetq_lane_s32(fVec, 0) && vgetq_lane_s32(fVec, 1)
                               && vgetq_lane_s32(fVec, 2) && vgetq_lane_s32(fVec, 3); }
    bool anyTrue() const { return vgetq_lane_s32(fVec, 0) || vgetq_lane_s32(fVec, 1)
                               || vgetq_lane_s32(fVec, 2) || vgetq_lane_s32(fVec, 3); }
private:
    int32x4_t fVec;
};

template <>
class SkNf<2, float> {
    typedef SkNi<2, int32_t> Ni;
public:
    SkNf(float32x2_t vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(float val)           : fVec(vdup_n_f32(val)) {}
    static SkNf Load(const float vals[2]) { return vld1_f32(vals); }
    SkNf(float a, float b) { fVec = (float32x2_t) { a, b }; }

    void store(float vals[2]) const { vst1_f32(vals, fVec); }

    SkNf approxInvert() const {
        float32x2_t est0 = vrecpe_f32(fVec),
                    est1 = vmul_f32(vrecps_f32(est0, fVec), est0);
        return est1;
    }
    SkNf invert() const {
        float32x2_t est1 = this->approxInvert().fVec,
                    est2 = vmul_f32(vrecps_f32(est1, fVec), est1);
        return est2;
    }

    SkNf operator + (const SkNf& o) const { return vadd_f32(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return vsub_f32(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return vmul_f32(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const {
    #if defined(SK_CPU_ARM64)
        return vdiv_f32(fVec, o.fVec);
    #else
        return vmul_f32(fVec, o.invert().fVec);
    #endif
    }

    Ni operator == (const SkNf& o) const { return vreinterpret_s32_u32(vceq_f32(fVec, o.fVec)); }
    Ni operator  < (const SkNf& o) const { return vreinterpret_s32_u32(vclt_f32(fVec, o.fVec)); }
    Ni operator  > (const SkNf& o) const { return vreinterpret_s32_u32(vcgt_f32(fVec, o.fVec)); }
    Ni operator <= (const SkNf& o) const { return vreinterpret_s32_u32(vcle_f32(fVec, o.fVec)); }
    Ni operator >= (const SkNf& o) const { return vreinterpret_s32_u32(vcge_f32(fVec, o.fVec)); }
    Ni operator != (const SkNf& o) const {
        return vreinterpret_s32_u32(vmvn_u32(vceq_f32(fVec, o.fVec)));
    }

    static SkNf Min(const SkNf& l, const SkNf& r) { return vmin_f32(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return vmax_f32(l.fVec, r.fVec); }

    SkNf rsqrt() const {
        float32x2_t est0 = vrsqrte_f32(fVec),
                    est1 = vmul_f32(vrsqrts_f32(fVec, vmul_f32(est0, est0)), est0);
        return est1;
    }

    SkNf sqrt() const {
    #if defined(SK_CPU_ARM64)
        return vsqrt_f32(fVec);
    #else
        float32x2_t est1 = this->rsqrt().fVec,
        // An extra step of Newton's method to refine the estimate of 1/sqrt(this).
                    est2 = vmul_f32(vrsqrts_f32(fVec, vmul_f32(est1, est1)), est1);
        return vmul_f32(fVec, est2);
    #endif
    }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 2);
        return vget_lane_f32(fVec, k&1);
    }

private:
    float32x2_t fVec;
};

#if defined(SK_CPU_ARM64)
template <>
class SkNi<2, int64_t> {
public:
    SkNi(int64x2_t vec) : fVec(vec) {}

    SkNi() {}
    bool allTrue() const { return vgetq_lane_s64(fVec, 0) && vgetq_lane_s64(fVec, 1); }
    bool anyTrue() const { return vgetq_lane_s64(fVec, 0) || vgetq_lane_s64(fVec, 1); }
private:
    int64x2_t fVec;
};

template <>
class SkNf<2, double> {
    typedef SkNi<2, int64_t> Ni;
public:
    SkNf(float64x2_t vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(double val)           : fVec(vdupq_n_f64(val))  {}
    static SkNf Load(const double vals[2]) { return vld1q_f64(vals); }
    SkNf(double a, double b) { fVec = (float64x2_t) { a, b }; }

    void store(double vals[2]) const { vst1q_f64(vals, fVec); }

    SkNf operator + (const SkNf& o) const { return vaddq_f64(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return vsubq_f64(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return vmulq_f64(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const { return vdivq_f64(fVec, o.fVec); }

    Ni operator == (const SkNf& o) const { return vreinterpretq_s64_u64(vceqq_f64(fVec, o.fVec)); }
    Ni operator  < (const SkNf& o) const { return vreinterpretq_s64_u64(vcltq_f64(fVec, o.fVec)); }
    Ni operator  > (const SkNf& o) const { return vreinterpretq_s64_u64(vcgtq_f64(fVec, o.fVec)); }
    Ni operator <= (const SkNf& o) const { return vreinterpretq_s64_u64(vcleq_f64(fVec, o.fVec)); }
    Ni operator >= (const SkNf& o) const { return vreinterpretq_s64_u64(vcgeq_f64(fVec, o.fVec)); }
    Ni operator != (const SkNf& o) const {
        return vreinterpretq_s64_u32(vmvnq_u32(vreinterpretq_u32_u64(vceqq_f64(fVec, o.fVec))));
    }

    static SkNf Min(const SkNf& l, const SkNf& r) { return vminq_f64(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return vmaxq_f64(l.fVec, r.fVec); }

    SkNf  sqrt() const { return vsqrtq_f64(fVec);  }
    SkNf rsqrt() const {
        float64x2_t est0 = vrsqrteq_f64(fVec),
                    est1 = vmulq_f64(vrsqrtsq_f64(fVec, vmulq_f64(est0, est0)), est0);
        return est1;
    }

    SkNf approxInvert() const {
        float64x2_t est0 = vrecpeq_f64(fVec),
                    est1 = vmulq_f64(vrecpsq_f64(est0, fVec), est0);
        return est1;
    }

    SkNf invert() const {
        float64x2_t est1 = this->approxInvert().fVec,
                    est2 = vmulq_f64(vrecpsq_f64(est1, fVec), est1),
                    est3 = vmulq_f64(vrecpsq_f64(est2, fVec), est2);
        return est3;
    }

    template <int k> double kth() const {
        SkASSERT(0 <= k && k < 2);
        return vgetq_lane_f64(fVec, k&1);
    }

private:
    float64x2_t fVec;
};
#endif//defined(SK_CPU_ARM64)

template <>
class SkNf<4, float> {
    typedef SkNi<4, int32_t> Ni;
public:
    SkNf(float32x4_t vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(float val)           : fVec(vdupq_n_f32(val)) {}
    static SkNf Load(const float vals[4]) { return vld1q_f32(vals); }
    SkNf(float a, float b, float c, float d) { fVec = (float32x4_t) { a, b, c, d }; }

    void store(float vals[4]) const { vst1q_f32(vals, fVec); }

    SkNf approxInvert() const {
        float32x4_t est0 = vrecpeq_f32(fVec),
                    est1 = vmulq_f32(vrecpsq_f32(est0, fVec), est0);
        return est1;
    }
    SkNf invert() const {
        float32x4_t est1 = this->approxInvert().fVec,
                    est2 = vmulq_f32(vrecpsq_f32(est1, fVec), est1);
        return est2;
    }

    SkNf operator + (const SkNf& o) const { return vaddq_f32(fVec, o.fVec); }
    SkNf operator - (const SkNf& o) const { return vsubq_f32(fVec, o.fVec); }
    SkNf operator * (const SkNf& o) const { return vmulq_f32(fVec, o.fVec); }
    SkNf operator / (const SkNf& o) const {
    #if defined(SK_CPU_ARM64)
        return vdivq_f32(fVec, o.fVec);
    #else
        return vmulq_f32(fVec, o.invert().fVec);
    #endif
    }

    Ni operator == (const SkNf& o) const { return vreinterpretq_s32_u32(vceqq_f32(fVec, o.fVec)); }
    Ni operator  < (const SkNf& o) const { return vreinterpretq_s32_u32(vcltq_f32(fVec, o.fVec)); }
    Ni operator  > (const SkNf& o) const { return vreinterpretq_s32_u32(vcgtq_f32(fVec, o.fVec)); }
    Ni operator <= (const SkNf& o) const { return vreinterpretq_s32_u32(vcleq_f32(fVec, o.fVec)); }
    Ni operator >= (const SkNf& o) const { return vreinterpretq_s32_u32(vcgeq_f32(fVec, o.fVec)); }
    Ni operator != (const SkNf& o) const {
        return vreinterpretq_s32_u32(vmvnq_u32(vceqq_f32(fVec, o.fVec)));
    }

    static SkNf Min(const SkNf& l, const SkNf& r) { return vminq_f32(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return vmaxq_f32(l.fVec, r.fVec); }

    SkNf rsqrt() const {
        float32x4_t est0 = vrsqrteq_f32(fVec),
                    est1 = vmulq_f32(vrsqrtsq_f32(fVec, vmulq_f32(est0, est0)), est0);
        return est1;
    }

    SkNf sqrt() const {
    #if defined(SK_CPU_ARM64)
        return vsqrtq_f32(fVec);
    #else
        float32x4_t est1 = this->rsqrt().fVec,
        // An extra step of Newton's method to refine the estimate of 1/sqrt(this).
                    est2 = vmulq_f32(vrsqrtsq_f32(fVec, vmulq_f32(est1, est1)), est1);
        return vmulq_f32(fVec, est2);
    #endif
    }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 4);
        return vgetq_lane_f32(fVec, k&3);
    }

protected:
    float32x4_t fVec;
};

#endif//SkNx_neon_DEFINED
