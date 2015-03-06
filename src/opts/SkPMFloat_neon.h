#include "SkColorPriv.h"
#include <arm_neon.h>

// For SkPMFloat(SkPMFColor), we widen our 8 bit components (fix8) to 8-bit components in 16 bits
// (fix8_16), then widen those to 8-bit-in-32-bits (fix8_32), and finally convert those to floats.

// get() and clamped() do the opposite, working from floats to 8-bit-in-32-bit,
// to 8-bit-in-16-bit, back down to 8-bit components.
// clamped() uses vqmovn to clamp while narrowing instead of just narrowing with vmovn.

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    SkPMColorAssert(c);
    uint8x8_t   fix8    = (uint8x8_t)vdup_n_u32(c);
    uint16x8_t  fix8_16 = vmovl_u8(fix8);
    uint32x4_t  fix8_32 = vmovl_u16(vget_low_u16(fix8_16));
    vst1q_f32(fColor, vcvtq_f32_u32(fix8_32));
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::get() const {
    SkASSERT(this->isValid());
    float32x4_t add_half = vaddq_f32(vld1q_f32(fColor), vdupq_n_f32(0.5f));
    uint32x4_t  fix8_32  = vcvtq_u32_f32(add_half);  // vcvtq_u32_f32 truncates, so round manually
    uint16x4_t  fix8_16  = vmovn_u32(fix8_32);
    uint8x8_t   fix8     = vmovn_u16(vcombine_u16(fix8_16, vdup_n_u16(0)));
    SkPMColor c = vget_lane_u32((uint32x2_t)fix8, 0);
    SkPMColorAssert(c);
    return c;
}

inline SkPMColor SkPMFloat::clamped() const {
    float32x4_t add_half = vaddq_f32(vld1q_f32(fColor), vdupq_n_f32(0.5f));
    uint32x4_t  fix8_32  = vcvtq_u32_f32(add_half);  // vcvtq_u32_f32 truncates, so round manually
    uint16x4_t  fix8_16  = vqmovn_u32(fix8_32);
    uint8x8_t   fix8     = vqmovn_u16(vcombine_u16(fix8_16, vdup_n_u16(0)));
    SkPMColor c = vget_lane_u32((uint32x2_t)fix8, 0);
    SkPMColorAssert(c);
    return c;
}

// TODO: we should be able to beat these loops on all three methods.
inline void SkPMFloat::From4PMColors(SkPMFloat floats[4], const SkPMColor colors[4]) {
    for (int i = 0; i < 4; i++) { floats[i] = FromPMColor(colors[i]); }
}

inline void SkPMFloat::To4PMColors(SkPMColor colors[4], const SkPMFloat floats[4]) {
    for (int i = 0; i < 4; i++) { colors[i] = floats[i].get(); }
}

inline void SkPMFloat::ClampTo4PMColors(SkPMColor colors[4], const SkPMFloat floats[4]) {
    for (int i = 0; i < 4; i++) { colors[i] = floats[i].clamped(); }
}
