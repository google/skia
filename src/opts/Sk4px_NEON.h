/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

inline Sk4px::Sk4px(SkPMColor px) : INHERITED((uint8x16_t)vdupq_n_u32(px)) {}

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

inline Sk4px::Wide Sk4px::mulWiden(const Sk16b& other) const {
    return Sk16h(vmull_u8(vget_low_u8 (this->fVec), vget_low_u8 (other.fVec)),
                 vmull_u8(vget_high_u8(this->fVec), vget_high_u8(other.fVec)));
}

inline Sk4px Sk4px::Wide::addNarrowHi(const Sk16h& other) const {
    const Sk4px::Wide o(other);  // Should be no code, but allows us to access fLo, fHi.
    return Sk16b(vcombine_u8(vaddhn_u16(this->fLo.fVec, o.fLo.fVec),
                             vaddhn_u16(this->fHi.fVec, o.fHi.fVec)));
}
