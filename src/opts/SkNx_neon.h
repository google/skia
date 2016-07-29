/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_neon_DEFINED
#define SkNx_neon_DEFINED

#include <arm_neon.h>

#define SKNX_IS_FAST

// ARMv8 has vrndmq_f32 to floor 4 floats.  Here we emulate it:
//   - roundtrip through integers via truncation
//   - subtract 1 if that's too big (possible for negative values).
// This restricts the domain of our inputs to a maximum somehwere around 2^31.  Seems plenty big.
static inline float32x4_t armv7_vrndmq_f32(float32x4_t v) {
    float32x4_t roundtrip = vcvtq_f32_s32(vcvtq_s32_f32(v));
    uint32x4_t too_big = roundtrip > v;
    return roundtrip - (float32x4_t)vandq_u32(too_big, (uint32x4_t)vdupq_n_f32(1));
}

template <>
class SkNx<2, float> {
public:
    SkNx(float32x2_t vec) : fVec(vec) {}

    SkNx() {}
    SkNx(float a, float b) : fVec{a,b} {}
    SkNx(float v)          : fVec{v,v} {}

    static SkNx Load(const void* ptr) { return vld1_f32((const float*)ptr); }
    void store(void* ptr) const { vst1_f32((float*)ptr, fVec); }

    SkNx operator + (const SkNx& o) const { return fVec + o.fVec; }
    SkNx operator - (const SkNx& o) const { return fVec - o.fVec; }
    SkNx operator * (const SkNx& o) const { return fVec * o.fVec; }
    SkNx operator / (const SkNx& o) const { return fVec / o.fVec; }

    SkNx operator == (const SkNx& o) const { return fVec == o.fVec; }
    SkNx operator  < (const SkNx& o) const { return fVec <  o.fVec; }
    SkNx operator  > (const SkNx& o) const { return fVec >  o.fVec; }
    SkNx operator <= (const SkNx& o) const { return fVec <= o.fVec; }
    SkNx operator >= (const SkNx& o) const { return fVec >= o.fVec; }
    SkNx operator != (const SkNx& o) const { return fVec != o.fVec; }

    static SkNx Min(const SkNx& l, const SkNx& r) { return vmin_f32(l.fVec, r.fVec); }
    static SkNx Max(const SkNx& l, const SkNx& r) { return vmax_f32(l.fVec, r.fVec); }

    SkNx rsqrt() const {
        float32x2_t est0 = vrsqrte_f32(fVec);
        return vmul_f32(vrsqrts_f32(fVec, vmul_f32(est0, est0)), est0);
    }

    SkNx sqrt() const {
    #if defined(SK_CPU_ARM64)
        return vsqrt_f32(fVec);
    #else
        float32x2_t est0 = vrsqrte_f32(fVec),
                    est1 = vmul_f32(vrsqrts_f32(fVec, vmul_f32(est0, est0)), est0),
                    est2 = vmul_f32(vrsqrts_f32(fVec, vmul_f32(est1, est1)), est1);
        return vmul_f32(fVec, est2);
    #endif
    }

    SkNx invert() const {
        float32x2_t est0 = vrecpe_f32(fVec),
                    est1 = vmul_f32(vrecps_f32(est0, fVec), est0);
        return est1;
    }

    float operator[](int k) const { return fVec[k&1]; }

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

template <>
class SkNx<4, float> {
public:
    SkNx(float32x4_t vec) : fVec(vec) {}

    SkNx() {}
    SkNx(float a, float b, float c, float d) : fVec{a,b,c,d} {}
    SkNx(float v)                            : fVec{v,v,v,v} {}

    static SkNx Load(const void* ptr) { return vld1q_f32((const float*)ptr); }
    void store(void* ptr) const { vst1q_f32((float*)ptr, fVec); }

    SkNx operator + (const SkNx& o) const { return fVec + o.fVec; }
    SkNx operator - (const SkNx& o) const { return fVec - o.fVec; }
    SkNx operator * (const SkNx& o) const { return fVec * o.fVec; }
    SkNx operator / (const SkNx& o) const { return fVec / o.fVec; }

    SkNx operator==(const SkNx& o) const { return fVec == o.fVec; }
    SkNx operator <(const SkNx& o) const { return fVec <  o.fVec; }
    SkNx operator >(const SkNx& o) const { return fVec >  o.fVec; }
    SkNx operator<=(const SkNx& o) const { return fVec <= o.fVec; }
    SkNx operator>=(const SkNx& o) const { return fVec >= o.fVec; }
    SkNx operator!=(const SkNx& o) const { return fVec != o.fVec; }

    static SkNx Min(const SkNx& l, const SkNx& r) { return vminq_f32(l.fVec, r.fVec); }
    static SkNx Max(const SkNx& l, const SkNx& r) { return vmaxq_f32(l.fVec, r.fVec); }

    SkNx abs() const { return vabsq_f32(fVec); }
    SkNx floor() const {
    #if defined(SK_CPU_ARM64)
        return vrndmq_f32(fVec);
    #else
        return armv7_vrndmq_f32(fVec);
    #endif
    }

    SkNx rsqrt() const {
        float32x4_t est0 = vrsqrteq_f32(fVec);
        return vmulq_f32(vrsqrtsq_f32(fVec, vmulq_f32(est0, est0)), est0);
    }

    SkNx sqrt() const {
    #if defined(SK_CPU_ARM64)
        return vsqrtq_f32(fVec);
    #else
        float32x4_t est0 = vrsqrteq_f32(fVec),
                    est1 = vmulq_f32(vrsqrtsq_f32(fVec, vmulq_f32(est0, est0)), est0),
                    est2 = vmulq_f32(vrsqrtsq_f32(fVec, vmulq_f32(est1, est1)), est1);
        return vmulq_f32(fVec, est2);
    #endif
    }

    SkNx invert() const {
        float32x4_t est0 = vrecpeq_f32(fVec),
                    est1 = vmulq_f32(vrecpsq_f32(est0, fVec), est0);
        return est1;
    }

    float operator[](int k) const { return fVec[k&3]; }

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

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return vbslq_f32(vreinterpretq_u32_f32(fVec), t.fVec, e.fVec);
    }

    float32x4_t fVec;
};

// It's possible that for our current use cases, representing this as
// half a uint16x8_t might be better than representing it as a uint16x4_t.
// It'd make conversion to Sk4b one step simpler.
template <>
class SkNx<4, uint16_t> {
public:
    SkNx(const uint16x4_t& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d) : fVec{a,b,c,d} {}
    SkNx(uint16_t v)                                     : fVec{v,v,v,v} {}

    static SkNx Load(const void* ptr) { return vld1_u16((const uint16_t*)ptr); }
    void store(void* ptr) const { vst1_u16((uint16_t*)ptr, fVec); }

    SkNx operator + (const SkNx& o) const { return fVec + o.fVec; }
    SkNx operator - (const SkNx& o) const { return fVec - o.fVec; }
    SkNx operator * (const SkNx& o) const { return fVec * o.fVec; }

    SkNx operator << (int bits) const { return fVec << SkNx(bits).fVec; }
    SkNx operator >> (int bits) const { return fVec >> SkNx(bits).fVec; }

    static SkNx Min(const SkNx& a, const SkNx& b) { return vmin_u16(a.fVec, b.fVec); }

    uint16_t operator[](int k) const { return fVec[k&3]; }

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return vbsl_u16(fVec, t.fVec, e.fVec);
    }

    uint16x4_t fVec;
};

template <>
class SkNx<8, uint16_t> {
public:
    SkNx(const uint16x8_t& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint16_t a, uint16_t b, uint16_t c, uint16_t d,
         uint16_t e, uint16_t f, uint16_t g, uint16_t h) : fVec{a,b,c,d,e,f,g,h} {}
    SkNx(uint16_t v)                                     : fVec{v,v,v,v,v,v,v,v} {}

    static SkNx Load(const void* ptr) { return vld1q_u16((const uint16_t*)ptr); }
    void store(void* ptr) const { vst1q_u16((uint16_t*)ptr, fVec); }

    SkNx operator + (const SkNx& o) const { return fVec + o.fVec; }
    SkNx operator - (const SkNx& o) const { return fVec - o.fVec; }
    SkNx operator * (const SkNx& o) const { return fVec * o.fVec; }

    SkNx operator << (int bits) const { return fVec << SkNx(bits).fVec; }
    SkNx operator >> (int bits) const { return fVec >> SkNx(bits).fVec; }

    static SkNx Min(const SkNx& a, const SkNx& b) { return vminq_u16(a.fVec, b.fVec); }

    uint16_t operator[](int k) const { return fVec[k&7]; }

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return vbslq_u16(fVec, t.fVec, e.fVec);
    }

    uint16x8_t fVec;
};

template <>
class SkNx<4, uint8_t> {
public:
    typedef uint32_t __attribute__((aligned(1))) unaligned_uint32_t;

    SkNx(const uint8x8_t& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : fVec{a,b,c,d,0,0,0,0} {}
    SkNx(uint8_t v)                                  : fVec{v,v,v,v,0,0,0,0} {}

    static SkNx Load(const void* ptr) {
        return (uint8x8_t)vld1_dup_u32((const unaligned_uint32_t*)ptr);
    }
    void store(void* ptr) const {
        return vst1_lane_u32((unaligned_uint32_t*)ptr, (uint32x2_t)fVec, 0);
    }

    uint8_t operator[](int k) const { return fVec[k&3]; }

    uint8x8_t fVec;
};

template <>
class SkNx<16, uint8_t> {
public:
    SkNx(const uint8x16_t& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
         uint8_t e, uint8_t f, uint8_t g, uint8_t h,
         uint8_t i, uint8_t j, uint8_t k, uint8_t l,
         uint8_t m, uint8_t n, uint8_t o, uint8_t p) : fVec{a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p} {}
    SkNx(uint8_t v)                                  : fVec{v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v} {}

    static SkNx Load(const void* ptr) { return vld1q_u8((const uint8_t*)ptr); }
    void store(void* ptr) const { vst1q_u8((uint8_t*)ptr, fVec); }

    SkNx saturatedAdd(const SkNx& o) const { return vqaddq_u8(fVec, o.fVec); }
    SkNx operator + (const SkNx& o) const { return fVec + o.fVec; }
    SkNx operator - (const SkNx& o) const { return fVec - o.fVec; }

    SkNx operator < (const SkNx& o) const { return fVec < o.fVec; }

    static SkNx Min(const SkNx& a, const SkNx& b) { return vminq_u8(a.fVec, b.fVec); }

    uint8_t operator[](int k) const { return fVec[k&15]; }

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return vbslq_u8(fVec, t.fVec, e.fVec);
    }

    uint8x16_t fVec;
};

template <>
class SkNx<4, int32_t> {
public:
    SkNx(const int32x4_t& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(int32_t a, int32_t b, int32_t c, int32_t d) : fVec{a,b,c,d} {}
    SkNx(int32_t v)                                  : fVec{v,v,v,v} {}

    static SkNx Load(const void* ptr) { return vld1q_s32((const int32_t*)ptr); }
    void store(void* ptr) const { return vst1q_s32((int32_t*)ptr, fVec); }

    SkNx operator + (const SkNx& o) const { return fVec + o.fVec; }
    SkNx operator - (const SkNx& o) const { return fVec - o.fVec; }
    SkNx operator * (const SkNx& o) const { return fVec * o.fVec; }

    SkNx operator & (const SkNx& o) const { return fVec & o.fVec; }
    SkNx operator | (const SkNx& o) const { return fVec | o.fVec; }
    SkNx operator ^ (const SkNx& o) const { return fVec ^ o.fVec; }

    SkNx operator << (int bits) const { return fVec << SkNx(bits).fVec; }
    SkNx operator >> (int bits) const { return fVec >> SkNx(bits).fVec; }

    SkNx operator == (const SkNx& o) const { return fVec == o.fVec; }
    SkNx operator <  (const SkNx& o) const { return fVec <  o.fVec; }
    SkNx operator >  (const SkNx& o) const { return fVec >  o.fVec; }

    static SkNx Min(const SkNx& a, const SkNx& b) { return vminq_s32(a.fVec, b.fVec); }

    int32_t operator[](int k) const { return fVec[k&3]; }

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return vbslq_s32(vreinterpretq_u32_s32(fVec), t.fVec, e.fVec);
    }

    int32x4_t fVec;
};

template <>
class SkNx<4, uint32_t> {
public:
    SkNx(const uint32x4_t& vec) : fVec(vec) {}

    SkNx() {}
    SkNx(uint32_t a, uint32_t b, uint32_t c, uint32_t d) : fVec{a,b,c,d} {}
    SkNx(uint32_t v)                                     : fVec{v,v,v,v} {}

    static SkNx Load(const void* ptr) { return vld1q_u32((const uint32_t*)ptr); }
    void store(void* ptr) const { return vst1q_u32((uint32_t*)ptr, fVec); }

    SkNx operator + (const SkNx& o) const { return fVec + o.fVec; }
    SkNx operator - (const SkNx& o) const { return fVec - o.fVec; }
    SkNx operator * (const SkNx& o) const { return fVec * o.fVec; }

    SkNx operator & (const SkNx& o) const { return fVec & o.fVec; }
    SkNx operator | (const SkNx& o) const { return fVec | o.fVec; }
    SkNx operator ^ (const SkNx& o) const { return fVec ^ o.fVec; }

    SkNx operator << (int bits) const { return fVec << SkNx(bits).fVec; }
    SkNx operator >> (int bits) const { return fVec >> SkNx(bits).fVec; }

    SkNx operator == (const SkNx& o) const { return fVec == o.fVec; }
    SkNx operator <  (const SkNx& o) const { return fVec <  o.fVec; }
    SkNx operator >  (const SkNx& o) const { return fVec >  o.fVec; }

    static SkNx Min(const SkNx& a, const SkNx& b) { return vminq_u32(a.fVec, b.fVec); }

    uint32_t operator[](int k) const { return fVec[k&3]; }

    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return vbslq_u32(fVec, t.fVec, e.fVec);
    }

    uint32x4_t fVec;
};

template<> inline Sk4i SkNx_cast<int32_t, float>(const Sk4f& src) {
    return vcvtq_s32_f32(src.fVec);

}
template<> inline Sk4f SkNx_cast<float, int32_t>(const Sk4i& src) {
    return vcvtq_f32_s32(src.fVec);
}
template<> inline Sk4f SkNx_cast<float, uint32_t>(const Sk4u& src) {
    return SkNx_cast<float>(Sk4i::Load(&src));
}

template<> inline Sk4h SkNx_cast<uint16_t, float>(const Sk4f& src) {
    return vqmovn_u32(vcvtq_u32_f32(src.fVec));
}

template<> inline Sk4f SkNx_cast<float, uint16_t>(const Sk4h& src) {
    return vcvtq_f32_u32(vmovl_u16(src.fVec));
}

template<> inline Sk4b SkNx_cast<uint8_t, float>(const Sk4f& src) {
    uint32x4_t _32 = vcvtq_u32_f32(src.fVec);
    uint16x4_t _16 = vqmovn_u32(_32);
    return vqmovn_u16(vcombine_u16(_16, _16));
}

template<> inline Sk4f SkNx_cast<float, uint8_t>(const Sk4b& src) {
    uint16x8_t _16 = vmovl_u8 (src.fVec) ;
    uint32x4_t _32 = vmovl_u16(vget_low_u16(_16));
    return vcvtq_f32_u32(_32);
}

template<> inline Sk16b SkNx_cast<uint8_t, float>(const Sk16f& src) {
    Sk8f ab, cd;
    SkNx_split(src, &ab, &cd);

    Sk4f a,b,c,d;
    SkNx_split(ab, &a, &b);
    SkNx_split(cd, &c, &d);
    return vuzpq_u8(vuzpq_u8((uint8x16_t)vcvtq_u32_f32(a.fVec),
                             (uint8x16_t)vcvtq_u32_f32(b.fVec)).val[0],
                    vuzpq_u8((uint8x16_t)vcvtq_u32_f32(c.fVec),
                             (uint8x16_t)vcvtq_u32_f32(d.fVec)).val[0]).val[0];
}

template<> inline Sk4h SkNx_cast<uint16_t, uint8_t>(const Sk4b& src) {
    return vget_low_u16(vmovl_u8(src.fVec));
}

template<> inline Sk4b SkNx_cast<uint8_t, uint16_t>(const Sk4h& src) {
    return vmovn_u16(vcombine_u16(src.fVec, src.fVec));
}

template<> inline Sk4b SkNx_cast<uint8_t, int32_t>(const Sk4i& src) {
    uint16x4_t _16 = vqmovun_s32(src.fVec);
    return vqmovn_u16(vcombine_u16(_16, _16));
}

template<> inline Sk4i SkNx_cast<int32_t, uint16_t>(const Sk4h& src) {
    return vreinterpretq_s32_u32(vmovl_u16(src.fVec));
}

template<> inline Sk4h SkNx_cast<uint16_t, int32_t>(const Sk4i& src) {
    return vmovn_u32(vreinterpretq_u32_s32(src.fVec));
}

static inline Sk4i Sk4f_round(const Sk4f& x) {
    return vcvtq_s32_f32((x + 0.5f).fVec);
}

static inline void Sk4h_load4(const void* ptr, Sk4h* r, Sk4h* g, Sk4h* b, Sk4h* a) {
    uint16x4x4_t rgba = vld4_u16((const uint16_t*)ptr);
    *r = rgba.val[0];
    *g = rgba.val[1];
    *b = rgba.val[2];
    *a = rgba.val[3];
}

static inline void Sk4h_store4(void* dst, const Sk4h& r, const Sk4h& g, const Sk4h& b,
                               const Sk4h& a) {
    uint16x4x4_t rgba = {{
        r.fVec,
        g.fVec,
        b.fVec,
        a.fVec,
    }};
    vst4_u16((uint16_t*) dst, rgba);
}

#endif//SkNx_neon_DEFINED
