/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace { // See SkPMFloat.h

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    SkPMColorAssert(c);
    uint8x8_t   fix8    = (uint8x8_t)vdup_n_u32(c);
    uint16x8_t  fix8_16 = vmovl_u8(fix8);
    uint32x4_t  fix8_32 = vmovl_u16(vget_low_u16(fix8_16));
    fVec = vmulq_f32(vcvtq_f32_u32(fix8_32), vdupq_n_f32(1.0f/255));
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::round() const {
    // vcvt_u32_f32 truncates, so we round manually by adding a half before converting.
    float32x4_t rounded = vmlaq_f32(vdupq_n_f32(0.5f), fVec, vdupq_n_f32(255));
    uint32x4_t  fix8_32 = vcvtq_u32_f32(rounded);
    uint16x4_t  fix8_16 = vqmovn_u32(fix8_32);
    uint8x8_t   fix8    = vqmovn_u16(vcombine_u16(fix8_16, vdup_n_u16(0)));
    SkPMColor c = vget_lane_u32((uint32x2_t)fix8, 0);
    SkPMColorAssert(c);
    return c;
}

inline Sk4f SkPMFloat::alphas() const {
    static_assert(SK_A32_SHIFT == 24, "Assuming little-endian.");
    return vdupq_lane_f32(vget_high_f32(fVec), 1);  // Duplicate high lane of high half i.e. lane 3.
}

}  // namespace
