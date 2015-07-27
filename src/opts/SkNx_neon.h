/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_neon_DEFINED
#define SkNx_neon_DEFINED

namespace {  // See SkNx.h

// Well, this is absurd.  The shifts require compile-time constant arguments.

#define SHIFT8(op, v, bits) switch(bits) { \
    case  1: return op(v,  1);  case  2: return op(v,  2);  case  3: return op(v,  3); \
    case  4: return op(v,  4);  case  5: return op(v,  5);  case  6: return op(v,  6); \
    case  7: return op(v,  7); \
    } return fVec

#define SHIFT16(op, v, bits) if (bits < 8) { SHIFT8(op, v, bits); } switch(bits) { \
                                case  8: return op(v,  8);  case  9: return op(v,  9); \
    case 10: return op(v, 10);  case 11: return op(v, 11);  case 12: return op(v, 12); \
    case 13: return op(v, 13);  case 14: return op(v, 14);  case 15: return op(v, 15); \
    } return fVec

#define SHIFT32(op, v, bits) if (bits < 16) { SHIFT16(op, v, bits); } switch(bits) { \
    case 16: return op(v, 16);  case 17: return op(v, 17);  case 18: return op(v, 18); \
    case 19: return op(v, 19);  case 20: return op(v, 20);  case 21: return op(v, 21); \
    case 22: return op(v, 22);  case 23: return op(v, 23);  case 24: return op(v, 24); \
    case 25: return op(v, 25);  case 26: return op(v, 26);  case 27: return op(v, 27); \
    case 28: return op(v, 28);  case 29: return op(v, 29);  case 30: return op(v, 30); \
    case 31: return op(v, 31); } return fVec

template <>
class SkNf<2, float> {
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

    SkNf operator == (const SkNf& o) const { return vreinterpret_f32_u32(vceq_f32(fVec, o.fVec)); }
    SkNf operator  < (const SkNf& o) const { return vreinterpret_f32_u32(vclt_f32(fVec, o.fVec)); }
    SkNf operator  > (const SkNf& o) const { return vreinterpret_f32_u32(vcgt_f32(fVec, o.fVec)); }
    SkNf operator <= (const SkNf& o) const { return vreinterpret_f32_u32(vcle_f32(fVec, o.fVec)); }
    SkNf operator >= (const SkNf& o) const { return vreinterpret_f32_u32(vcge_f32(fVec, o.fVec)); }
    SkNf operator != (const SkNf& o) const {
        return vreinterpret_f32_u32(vmvn_u32(vceq_f32(fVec, o.fVec)));
    }

    static SkNf Min(const SkNf& l, const SkNf& r) { return vmin_f32(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return vmax_f32(l.fVec, r.fVec); }

    SkNf rsqrt0() const { return vrsqrte_f32(fVec); }
    SkNf rsqrt1() const {
        float32x2_t est0 = this->rsqrt0().fVec;
        return vmul_f32(vrsqrts_f32(fVec, vmul_f32(est0, est0)), est0);
    }
    SkNf rsqrt2() const {
        float32x2_t est1 = this->rsqrt1().fVec;
        return vmul_f32(vrsqrts_f32(fVec, vmul_f32(est1, est1)), est1);
    }

    SkNf sqrt() const {
    #if defined(SK_CPU_ARM64)
        return vsqrt_f32(fVec);
    #else
        return *this * this->rsqrt2();
    #endif
    }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 2);
        return vget_lane_f32(fVec, k&1);
    }

    bool allTrue() const {
        auto v = vreinterpret_u32_f32(fVec);
        return vget_lane_u32(v,0) && vget_lane_u32(v,1);
    }
    bool anyTrue() const {
        auto v = vreinterpret_u32_f32(fVec);
        return vget_lane_u32(v,0) || vget_lane_u32(v,1);
    }

    float32x2_t fVec;
};

#if defined(SK_CPU_ARM64)
template <>
class SkNf<2, double> {
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

    // vreinterpretq_f64_u64 and vreinterpretq_f64_u32 don't seem to exist....  weird.
    SkNf operator==(const SkNf& o) const { return (float64x2_t)(vceqq_f64(fVec, o.fVec)); }
    SkNf operator <(const SkNf& o) const { return (float64x2_t)(vcltq_f64(fVec, o.fVec)); }
    SkNf operator >(const SkNf& o) const { return (float64x2_t)(vcgtq_f64(fVec, o.fVec)); }
    SkNf operator<=(const SkNf& o) const { return (float64x2_t)(vcleq_f64(fVec, o.fVec)); }
    SkNf operator>=(const SkNf& o) const { return (float64x2_t)(vcgeq_f64(fVec, o.fVec)); }
    SkNf operator != (const SkNf& o) const {
        return (float64x2_t)(vmvnq_u32(vreinterpretq_u32_u64(vceqq_f64(fVec, o.fVec))));
    }

    static SkNf Min(const SkNf& l, const SkNf& r) { return vminq_f64(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return vmaxq_f64(l.fVec, r.fVec); }

    SkNf  sqrt() const { return vsqrtq_f64(fVec);  }

    SkNf rsqrt0() const { return vrsqrteq_f64(fVec); }
    SkNf rsqrt1() const {
        float64x2_t est0 = this->rsqrt0().fVec;
        return vmulq_f64(vrsqrtsq_f64(fVec, vmulq_f64(est0, est0)), est0);
    }
    SkNf rsqrt2() const {
        float64x2_t est1 = this->rsqrt1().fVec;
        return vmulq_f64(vrsqrtsq_f64(fVec, vmulq_f64(est1, est1)), est1);
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

    // vreinterpretq_u64_f64 doesn't seem to exist....  weird.
    bool allTrue() const {
        auto v = (uint64x2_t)(fVec);
        return vgetq_lane_u64(v,0) && vgetq_lane_u64(v,1);
    }
    bool anyTrue() const {
        auto v = (uint64x2_t)(fVec);
        return vgetq_lane_u64(v,0) || vgetq_lane_u64(v,1);
    }

    float64x2_t fVec;
};
#endif//defined(SK_CPU_ARM64)

template <>
class SkNi<4, int> {
public:
    SkNi(const int32x4_t& vec) : fVec(vec) {}

    SkNi() {}
    explicit SkNi(int val) : fVec(vdupq_n_s32(val)) {}
    static SkNi Load(const int vals[4]) { return vld1q_s32(vals); }
    SkNi(int a, int b, int c, int d) { fVec = (int32x4_t) { a, b, c, d }; }

    void store(int vals[4]) const { vst1q_s32(vals, fVec); }

    SkNi operator + (const SkNi& o) const { return vaddq_s32(fVec, o.fVec); }
    SkNi operator - (const SkNi& o) const { return vsubq_s32(fVec, o.fVec); }
    SkNi operator * (const SkNi& o) const { return vmulq_s32(fVec, o.fVec); }

    SkNi operator << (int bits) const { SHIFT32(vshlq_n_s32, fVec, bits); }
    SkNi operator >> (int bits) const { SHIFT32(vshrq_n_s32, fVec, bits); }

    template <int k> int kth() const {
        SkASSERT(0 <= k && k < 4);
        return vgetq_lane_s32(fVec, k&3);
    }

    int32x4_t fVec;
};

template <>
class SkNf<4, float> {
public:
    SkNf(float32x4_t vec) : fVec(vec) {}

    SkNf() {}
    explicit SkNf(float val)           : fVec(vdupq_n_f32(val)) {}
    static SkNf Load(const float vals[4]) { return vld1q_f32(vals); }
    SkNf(float a, float b, float c, float d) { fVec = (float32x4_t) { a, b, c, d }; }

    void store(float vals[4]) const { vst1q_f32(vals, fVec); }

    SkNi<4, int> castTrunc() const { return vcvtq_s32_f32(fVec); }

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

    SkNf operator==(const SkNf& o) const { return vreinterpretq_f32_u32(vceqq_f32(fVec, o.fVec)); }
    SkNf operator <(const SkNf& o) const { return vreinterpretq_f32_u32(vcltq_f32(fVec, o.fVec)); }
    SkNf operator >(const SkNf& o) const { return vreinterpretq_f32_u32(vcgtq_f32(fVec, o.fVec)); }
    SkNf operator<=(const SkNf& o) const { return vreinterpretq_f32_u32(vcleq_f32(fVec, o.fVec)); }
    SkNf operator>=(const SkNf& o) const { return vreinterpretq_f32_u32(vcgeq_f32(fVec, o.fVec)); }
    SkNf operator!=(const SkNf& o) const {
        return vreinterpretq_f32_u32(vmvnq_u32(vceqq_f32(fVec, o.fVec)));
    }

    static SkNf Min(const SkNf& l, const SkNf& r) { return vminq_f32(l.fVec, r.fVec); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return vmaxq_f32(l.fVec, r.fVec); }

    SkNf rsqrt0() const { return vrsqrteq_f32(fVec); }
    SkNf rsqrt1() const {
        float32x4_t est0 = this->rsqrt0().fVec;
        return vmulq_f32(vrsqrtsq_f32(fVec, vmulq_f32(est0, est0)), est0);
    }
    SkNf rsqrt2() const {
        float32x4_t est1 = this->rsqrt1().fVec;
        return vmulq_f32(vrsqrtsq_f32(fVec, vmulq_f32(est1, est1)), est1);
    }

    SkNf sqrt() const {
    #if defined(SK_CPU_ARM64)
        return vsqrtq_f32(fVec);
    #else
        return *this * this->rsqrt2();
    #endif
    }

    template <int k> float kth() const {
        SkASSERT(0 <= k && k < 4);
        return vgetq_lane_f32(fVec, k&3);
    }

    bool allTrue() const {
        auto v = vreinterpretq_u32_f32(fVec);
        return vgetq_lane_u32(v,0) && vgetq_lane_u32(v,1)
            && vgetq_lane_u32(v,2) && vgetq_lane_u32(v,3);
    }
    bool anyTrue() const {
        auto v = vreinterpretq_u32_f32(fVec);
        return vgetq_lane_u32(v,0) || vgetq_lane_u32(v,1)
            || vgetq_lane_u32(v,2) || vgetq_lane_u32(v,3);
    }

    SkNf thenElse(const SkNf& t, const SkNf& e) const {
        return vbslq_f32(vreinterpretq_u32_f32(fVec), t.fVec, e.fVec);
    }

    float32x4_t fVec;
};

template <>
class SkNi<8, uint16_t> {
public:
    SkNi(const uint16x8_t& vec) : fVec(vec) {}

    SkNi() {}
    explicit SkNi(uint16_t val) : fVec(vdupq_n_u16(val)) {}
    static SkNi Load(const uint16_t vals[8]) { return vld1q_u16(vals); }

    SkNi(uint16_t a, uint16_t b, uint16_t c, uint16_t d,
         uint16_t e, uint16_t f, uint16_t g, uint16_t h) {
        fVec = (uint16x8_t) { a,b,c,d, e,f,g,h };
    }

    void store(uint16_t vals[8]) const { vst1q_u16(vals, fVec); }

    SkNi operator + (const SkNi& o) const { return vaddq_u16(fVec, o.fVec); }
    SkNi operator - (const SkNi& o) const { return vsubq_u16(fVec, o.fVec); }
    SkNi operator * (const SkNi& o) const { return vmulq_u16(fVec, o.fVec); }

    SkNi operator << (int bits) const { SHIFT16(vshlq_n_u16, fVec, bits); }
    SkNi operator >> (int bits) const { SHIFT16(vshrq_n_u16, fVec, bits); }

    static SkNi Min(const SkNi& a, const SkNi& b) { return vminq_u16(a.fVec, b.fVec); }

    template <int k> uint16_t kth() const {
        SkASSERT(0 <= k && k < 8);
        return vgetq_lane_u16(fVec, k&7);
    }

    SkNi thenElse(const SkNi& t, const SkNi& e) const {
        return vbslq_u16(fVec, t.fVec, e.fVec);
    }

    uint16x8_t fVec;
};

template <>
class SkNi<16, uint8_t> {
public:
    SkNi(const uint8x16_t& vec) : fVec(vec) {}

    SkNi() {}
    explicit SkNi(uint8_t val) : fVec(vdupq_n_u8(val)) {}
    static SkNi Load(const uint8_t vals[16]) { return vld1q_u8(vals); }

    SkNi(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
         uint8_t e, uint8_t f, uint8_t g, uint8_t h,
         uint8_t i, uint8_t j, uint8_t k, uint8_t l,
         uint8_t m, uint8_t n, uint8_t o, uint8_t p) {
        fVec = (uint8x16_t) { a,b,c,d, e,f,g,h, i,j,k,l, m,n,o,p };
    }

    void store(uint8_t vals[16]) const { vst1q_u8(vals, fVec); }

    SkNi saturatedAdd(const SkNi& o) const { return vqaddq_u8(fVec, o.fVec); }

    SkNi operator + (const SkNi& o) const { return vaddq_u8(fVec, o.fVec); }
    SkNi operator - (const SkNi& o) const { return vsubq_u8(fVec, o.fVec); }

    static SkNi Min(const SkNi& a, const SkNi& b) { return vminq_u8(a.fVec, b.fVec); }
    SkNi operator < (const SkNi& o) const { return vcltq_u8(fVec, o.fVec); }

    template <int k> uint8_t kth() const {
        SkASSERT(0 <= k && k < 15);
        return vgetq_lane_u8(fVec, k&16);
    }

    SkNi thenElse(const SkNi& t, const SkNi& e) const {
        return vbslq_u8(fVec, t.fVec, e.fVec);
    }

    uint8x16_t fVec;
};

#undef SHIFT32
#undef SHIFT16
#undef SHIFT8

}  // namespace

#endif//SkNx_neon_DEFINED
