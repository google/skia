/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPx_neon_DEFINED
#define SkPx_neon_DEFINED

// When we have NEON, we like to work 8 pixels at a time.
// This lets us exploit vld4/vst4 and represent SkPx as planar uint8x8x4_t,
// Wide as planar uint16x8x4_t, and Alpha as a single uint8x8_t plane.

namespace neon {

struct SkPx {
    static const int N = 8;

    uint8x8x4_t fVec;
    SkPx(uint8x8x4_t vec) : fVec(vec) {}

    static SkPx Dup(uint32_t px) { return vld4_dup_u8((const uint8_t*)&px); }
    static SkPx Load(const uint32_t* px) { return vld4_u8((const uint8_t*)px); }
    static SkPx Load(const uint32_t* px, int n) {
        SkASSERT(0 < n && n < 8);
        uint8x8x4_t v = vld4_dup_u8((const uint8_t*)px);  // n>=1, so start all lanes with pixel 0.
        switch (n) {
            case 7: v = vld4_lane_u8((const uint8_t*)(px+6), v, 6);  // fall through
            case 6: v = vld4_lane_u8((const uint8_t*)(px+5), v, 5);  // fall through
            case 5: v = vld4_lane_u8((const uint8_t*)(px+4), v, 4);  // fall through
            case 4: v = vld4_lane_u8((const uint8_t*)(px+3), v, 3);  // fall through
            case 3: v = vld4_lane_u8((const uint8_t*)(px+2), v, 2);  // fall through
            case 2: v = vld4_lane_u8((const uint8_t*)(px+1), v, 1);
        }
        return v;
    }

    void store(uint32_t* px) const { vst4_u8((uint8_t*)px, fVec); }
    void store(uint32_t* px, int n) const {
        SkASSERT(0 < n && n < 8);
        switch (n) {
            case 7: vst4_lane_u8((uint8_t*)(px+6), fVec, 6);
            case 6: vst4_lane_u8((uint8_t*)(px+5), fVec, 5);
            case 5: vst4_lane_u8((uint8_t*)(px+4), fVec, 4);
            case 4: vst4_lane_u8((uint8_t*)(px+3), fVec, 3);
            case 3: vst4_lane_u8((uint8_t*)(px+2), fVec, 2);
            case 2: vst4_lane_u8((uint8_t*)(px+1), fVec, 1);
            case 1: vst4_lane_u8((uint8_t*)(px+0), fVec, 0);
        }
    }

    struct Alpha {
        uint8x8_t fA;
        Alpha(uint8x8_t a) : fA(a) {}

        static Alpha Dup(uint8_t a) { return vdup_n_u8(a); }
        static Alpha Load(const uint8_t* a) { return vld1_u8(a); }
        static Alpha Load(const uint8_t* a, int n) {
            SkASSERT(0 < n && n < 8);
            uint8x8_t v = vld1_dup_u8(a);  // n>=1, so start all lanes with alpha 0.
            switch (n) {
                case 7: v = vld1_lane_u8(a+6, v, 6);  // fall through
                case 6: v = vld1_lane_u8(a+5, v, 5);  // fall through
                case 5: v = vld1_lane_u8(a+4, v, 4);  // fall through
                case 4: v = vld1_lane_u8(a+3, v, 3);  // fall through
                case 3: v = vld1_lane_u8(a+2, v, 2);  // fall through
                case 2: v = vld1_lane_u8(a+1, v, 1);
            }
            return v;
        }
        Alpha inv() const { return vsub_u8(vdup_n_u8(255), fA); }
    };

    struct Wide {
        uint16x8x4_t fVec;
        Wide(uint16x8x4_t vec) : fVec(vec) {}

        Wide operator+(const Wide& o) const {
            return (uint16x8x4_t) {{
                vaddq_u16(fVec.val[0], o.fVec.val[0]),
                vaddq_u16(fVec.val[1], o.fVec.val[1]),
                vaddq_u16(fVec.val[2], o.fVec.val[2]),
                vaddq_u16(fVec.val[3], o.fVec.val[3]),
            }};
        }
        Wide operator-(const Wide& o) const {
            return (uint16x8x4_t) {{
                vsubq_u16(fVec.val[0], o.fVec.val[0]),
                vsubq_u16(fVec.val[1], o.fVec.val[1]),
                vsubq_u16(fVec.val[2], o.fVec.val[2]),
                vsubq_u16(fVec.val[3], o.fVec.val[3]),
            }};
        }

        template <int bits> Wide shl() const {
            return (uint16x8x4_t) {{
                vshlq_n_u16(fVec.val[0], bits),
                vshlq_n_u16(fVec.val[1], bits),
                vshlq_n_u16(fVec.val[2], bits),
                vshlq_n_u16(fVec.val[3], bits),
            }};
        }
        template <int bits> Wide shr() const {
            return (uint16x8x4_t) {{
                vshrq_n_u16(fVec.val[0], bits),
                vshrq_n_u16(fVec.val[1], bits),
                vshrq_n_u16(fVec.val[2], bits),
                vshrq_n_u16(fVec.val[3], bits),
            }};
        }

        SkPx addNarrowHi(const SkPx& o) const {
            return (uint8x8x4_t) {{
                vshrn_n_u16(vaddw_u8(fVec.val[0], o.fVec.val[0]), 8),
                vshrn_n_u16(vaddw_u8(fVec.val[1], o.fVec.val[1]), 8),
                vshrn_n_u16(vaddw_u8(fVec.val[2], o.fVec.val[2]), 8),
                vshrn_n_u16(vaddw_u8(fVec.val[3], o.fVec.val[3]), 8),
            }};
        }
    };

    Alpha alpha() const { return fVec.val[3]; }

    Wide widenLo() const {
        return (uint16x8x4_t) {{
            vmovl_u8(fVec.val[0]),
            vmovl_u8(fVec.val[1]),
            vmovl_u8(fVec.val[2]),
            vmovl_u8(fVec.val[3]),
        }};
    }
    // TODO: these two can probably be done faster.
    Wide widenHi() const { return this->widenLo().shl<8>(); }
    Wide widenLoHi() const { return this->widenLo() + this->widenHi(); }

    SkPx operator+(const SkPx& o) const {
        return (uint8x8x4_t) {{
            vadd_u8(fVec.val[0], o.fVec.val[0]),
            vadd_u8(fVec.val[1], o.fVec.val[1]),
            vadd_u8(fVec.val[2], o.fVec.val[2]),
            vadd_u8(fVec.val[3], o.fVec.val[3]),
        }};
    }
    SkPx operator-(const SkPx& o) const {
        return (uint8x8x4_t) {{
            vsub_u8(fVec.val[0], o.fVec.val[0]),
            vsub_u8(fVec.val[1], o.fVec.val[1]),
            vsub_u8(fVec.val[2], o.fVec.val[2]),
            vsub_u8(fVec.val[3], o.fVec.val[3]),
        }};
    }
    SkPx saturatedAdd(const SkPx& o) const {
        return (uint8x8x4_t) {{
            vqadd_u8(fVec.val[0], o.fVec.val[0]),
            vqadd_u8(fVec.val[1], o.fVec.val[1]),
            vqadd_u8(fVec.val[2], o.fVec.val[2]),
            vqadd_u8(fVec.val[3], o.fVec.val[3]),
        }};
    }

    Wide operator*(const Alpha& a) const {
        return (uint16x8x4_t) {{
            vmull_u8(fVec.val[0], a.fA),
            vmull_u8(fVec.val[1], a.fA),
            vmull_u8(fVec.val[2], a.fA),
            vmull_u8(fVec.val[3], a.fA),
        }};
    }
    SkPx approxMulDiv255(const Alpha& a) const {
        return (*this * a).addNarrowHi(*this);
    }

    SkPx addAlpha(const Alpha& a) const {
        return (uint8x8x4_t) {{
            fVec.val[0],
            fVec.val[1],
            fVec.val[2],
            vadd_u8(fVec.val[3], a.fA),
        }};
    }
};

}  // namespace neon

typedef neon::SkPx SkPx;

#endif//SkPx_neon_DEFINED
