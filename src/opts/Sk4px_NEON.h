/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace { // See Sk4px.h

inline Sk4px Sk4px::DupPMColor(SkPMColor px) { return Sk16b((uint8x16_t)vdupq_n_u32(px)); }

inline Sk4px Sk4px::Load4(const SkPMColor px[4]) {
    return Sk16b((uint8x16_t)vld1q_u32(px));
}
inline Sk4px Sk4px::Load2(const SkPMColor px[2]) {
    uint32x2_t px2 = vld1_u32(px);
    return Sk16b((uint8x16_t)vcombine_u32(px2, px2));
}
inline Sk4px Sk4px::Load1(const SkPMColor px[1]) {
    return Sk16b((uint8x16_t)vdupq_n_u32(*px));
}

inline void Sk4px::store4(SkPMColor px[4]) const {
    vst1q_u32(px, (uint32x4_t)this->fVec);
}
inline void Sk4px::store2(SkPMColor px[2]) const {
    vst1_u32(px, (uint32x2_t)vget_low_u8(this->fVec));
}
inline void Sk4px::store1(SkPMColor px[1]) const {
    vst1q_lane_u32(px, (uint32x4_t)this->fVec, 0);
}

inline Sk4px::Wide Sk4px::widenLo() const {
    return Sk16h(vmovl_u8(vget_low_u8 (this->fVec)),
                 vmovl_u8(vget_high_u8(this->fVec)));
}

inline Sk4px::Wide Sk4px::widenHi() const {
    return Sk16h(vshll_n_u8(vget_low_u8 (this->fVec), 8),
                 vshll_n_u8(vget_high_u8(this->fVec), 8));
}

inline Sk4px::Wide Sk4px::widenLoHi() const {
    auto zipped = vzipq_u8(this->fVec, this->fVec);
    return Sk16h((uint16x8_t)zipped.val[0],
                 (uint16x8_t)zipped.val[1]);
}

inline Sk4px::Wide Sk4px::mulWiden(const Sk16b& other) const {
    return Sk16h(vmull_u8(vget_low_u8 (this->fVec), vget_low_u8 (other.fVec)),
                 vmull_u8(vget_high_u8(this->fVec), vget_high_u8(other.fVec)));
}

inline Sk4px Sk4px::Wide::addNarrowHi(const Sk16h& other) const {
    const Sk4px::Wide o(other);  // Should be no code, but allows us to access fLo, fHi.
    return Sk16b(vcombine_u8(vaddhn_u16(this->fLo.fVec, o.fLo.fVec),
                             vaddhn_u16(this->fHi.fVec, o.fHi.fVec)));
}

inline Sk4px Sk4px::alphas() const {
    auto as = vshrq_n_u32((uint32x4_t)fVec, SK_A32_SHIFT);  // ___3 ___2 ___1 ___0
    return Sk16b((uint8x16_t)vmulq_n_u32(as, 0x01010101));  // 3333 2222 1111 0000
}

inline Sk4px Sk4px::Load4Alphas(const SkAlpha a[4]) {
    uint8x16_t a8 = vdupq_n_u8(0);                           // ____ ____ ____ ____
    a8 = vld1q_lane_u8(a+0, a8,  0);                         // ____ ____ ____ ___0
    a8 = vld1q_lane_u8(a+1, a8,  4);                         // ____ ____ ___1 ___0
    a8 = vld1q_lane_u8(a+2, a8,  8);                         // ____ ___2 ___1 ___0
    a8 = vld1q_lane_u8(a+3, a8, 12);                         // ___3 ___2 ___1 ___0
    auto a32 = (uint32x4_t)a8;                               //
    return Sk16b((uint8x16_t)vmulq_n_u32(a32, 0x01010101));  // 3333 2222 1111 0000
}

inline Sk4px Sk4px::Load2Alphas(const SkAlpha a[2]) {
    uint8x16_t a8 = vdupq_n_u8(0);                           // ____ ____ ____ ____
    a8 = vld1q_lane_u8(a+0, a8,  0);                         // ____ ____ ____ ___0
    a8 = vld1q_lane_u8(a+1, a8,  4);                         // ____ ____ ___1 ___0
    auto a32 = (uint32x4_t)a8;                               //
    return Sk16b((uint8x16_t)vmulq_n_u32(a32, 0x01010101));  // ____ ____ 1111 0000
}

inline Sk4px Sk4px::zeroColors() const {
    return Sk16b(vandq_u8(this->fVec, (uint8x16_t)vdupq_n_u32(0xFF << SK_A32_SHIFT)));
}

inline Sk4px Sk4px::zeroAlphas() const {
    // vbic(a,b) == a & ~b
    return Sk16b(vbicq_u8(this->fVec, (uint8x16_t)vdupq_n_u32(0xFF << SK_A32_SHIFT)));
}

static inline uint8x16_t widen_to_8888(uint16x4_t v) {
    // RGB565 format:   |R....|G.....|B....|
    //           Bit:  16    11      5     0

    // First get each pixel into its own 32-bit lane.
    //      v ==                       rgb3 rgb2  rgb1 rgb0
    // spread == 0000 rgb3  0000 rgb2  0000 rgb1  0000 rgb0
    uint32x4_t spread = vmovl_u16(v);

    // Get each color independently, still in 565 precison but down at bit 0.
    auto r5 = vshrq_n_u32(spread, 11),
         g6 = vandq_u32(vdupq_n_u32(63), vshrq_n_u32(spread, 5)),
         b5 = vandq_u32(vdupq_n_u32(31), spread);

    // Scale 565 precision up to 8-bit each, filling low 323 bits with high bits of each component.
    auto r8 = vorrq_u32(vshlq_n_u32(r5, 3), vshrq_n_u32(r5, 2)),
         g8 = vorrq_u32(vshlq_n_u32(g6, 2), vshrq_n_u32(g6, 4)),
         b8 = vorrq_u32(vshlq_n_u32(b5, 3), vshrq_n_u32(b5, 2));

    // Now put all the 8-bit components into SkPMColor order.
    return (uint8x16_t)vorrq_u32(vshlq_n_u32(r8, SK_R32_SHIFT),   // TODO: one shift is zero...
                       vorrq_u32(vshlq_n_u32(g8, SK_G32_SHIFT),
                       vorrq_u32(vshlq_n_u32(b8, SK_B32_SHIFT),
                                 vdupq_n_u32(0xFF << SK_A32_SHIFT))));
}

static inline uint16x4_t narrow_to_565(uint8x16_t w8x16) {
    uint32x4_t w = (uint32x4_t)w8x16;

    // Extract out top RGB 565 bits of each pixel, with no rounding.
    auto r5 = vandq_u32(vdupq_n_u32(31), vshrq_n_u32(w, SK_R32_SHIFT + 3)),
         g6 = vandq_u32(vdupq_n_u32(63), vshrq_n_u32(w, SK_G32_SHIFT + 2)),
         b5 = vandq_u32(vdupq_n_u32(31), vshrq_n_u32(w, SK_B32_SHIFT + 3));

    // Now put the bits in place in the low 16-bits of each 32-bit lane.
    auto spread = vorrq_u32(vshlq_n_u32(r5, 11),
                  vorrq_u32(vshlq_n_u32(g6,  5),
                            b5));

    // Pack the low 16-bits of our 128-bit register down into a 64-bit register.
    // spread == 0000 rgb3  0000 rgb2  0000 rgb1  0000 rgb0
    //      v ==                       rgb3 rgb2  rgb1 rgb0
    auto v = vmovn_u32(spread);
    return v;
}


inline Sk4px Sk4px::Load4(const SkPMColor16 src[4]) {
    return Sk16b(widen_to_8888(vld1_u16(src)));
}
inline Sk4px Sk4px::Load2(const SkPMColor16 src[2]) {
    auto src2 = ((uint32_t)src[0]      )
              | ((uint32_t)src[1] << 16);
    return Sk16b(widen_to_8888(vcreate_u16(src2)));
}
inline Sk4px Sk4px::Load1(const SkPMColor16 src[1]) {
    return Sk16b(widen_to_8888(vcreate_u16(src[0])));
}

inline void Sk4px::store4(SkPMColor16 dst[4]) const {
    vst1_u16(dst, narrow_to_565(this->fVec));
}
inline void Sk4px::store2(SkPMColor16 dst[2]) const {
    auto v = narrow_to_565(this->fVec);
    dst[0] = vget_lane_u16(v, 0);
    dst[1] = vget_lane_u16(v, 1);
}
inline void Sk4px::store1(SkPMColor16 dst[1]) const {
    dst[0] = vget_lane_u16(narrow_to_565(this->fVec), 0);
}

} // namespace

