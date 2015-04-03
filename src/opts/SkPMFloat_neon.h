/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// For SkPMFloat(SkPMFColor), we widen our 8 bit components (fix8) to 8-bit components in 16 bits
// (fix8_16), then widen those to 8-bit-in-32-bits (fix8_32), and finally convert those to floats.

// round() and roundClamp() do the opposite, working from floats to 8-bit-in-32-bit,
// to 8-bit-in-16-bit, back down to 8-bit components.
// roundClamp() uses vqmovn to clamp while narrowing instead of just narrowing with vmovn.

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    SkPMColorAssert(c);
    uint8x8_t   fix8    = (uint8x8_t)vdup_n_u32(c);
    uint16x8_t  fix8_16 = vmovl_u8(fix8);
    uint32x4_t  fix8_32 = vmovl_u16(vget_low_u16(fix8_16));
    fVec = vcvtq_f32_u32(fix8_32);
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::trunc() const {
    uint32x4_t  fix8_32  = vcvtq_u32_f32(fVec);  // vcvtq_u32_f32 truncates
    uint16x4_t  fix8_16  = vmovn_u32(fix8_32);
    uint8x8_t   fix8     = vmovn_u16(vcombine_u16(fix8_16, vdup_n_u16(0)));
    SkPMColor c = vget_lane_u32((uint32x2_t)fix8, 0);
    SkPMColorAssert(c);
    return c;
}

inline SkPMColor SkPMFloat::round() const {
    return SkPMFloat(Sk4f(0.5f) + *this).trunc();
}

inline SkPMColor SkPMFloat::roundClamp() const {
    float32x4_t add_half = vaddq_f32(fVec, vdupq_n_f32(0.5f));
    uint32x4_t  fix8_32  = vcvtq_u32_f32(add_half);  // vcvtq_u32_f32 truncates, so round manually
    uint16x4_t  fix8_16  = vqmovn_u32(fix8_32);
    uint8x8_t   fix8     = vqmovn_u16(vcombine_u16(fix8_16, vdup_n_u16(0)));
    SkPMColor c = vget_lane_u32((uint32x2_t)fix8, 0);
    SkPMColorAssert(c);
    return c;
}

// TODO: we should be able to beat these loops on all three methods.
inline void SkPMFloat::From4PMColors(const SkPMColor colors[4],
                                     SkPMFloat* a, SkPMFloat* b, SkPMFloat* c, SkPMFloat* d) {
    *a = FromPMColor(colors[0]);
    *b = FromPMColor(colors[1]);
    *c = FromPMColor(colors[2]);
    *d = FromPMColor(colors[3]);
}

inline void SkPMFloat::RoundTo4PMColors(
        const SkPMFloat& a, const SkPMFloat& b, const SkPMFloat&c, const SkPMFloat& d,
        SkPMColor colors[4]) {
    colors[0] = a.round();
    colors[1] = b.round();
    colors[2] = c.round();
    colors[3] = d.round();
}

inline void SkPMFloat::RoundClampTo4PMColors(
        const SkPMFloat& a, const SkPMFloat& b, const SkPMFloat&c, const SkPMFloat& d,
        SkPMColor colors[4]) {
    colors[0] = a.roundClamp();
    colors[1] = b.roundClamp();
    colors[2] = c.roundClamp();
    colors[3] = d.roundClamp();
}
