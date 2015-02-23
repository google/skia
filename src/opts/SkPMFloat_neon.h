#include "SkColorPriv.h"
#include "SkPMFloat.h"
#include <arm_neon.h>

// For set(), we widen our 8 bit components (fix8) to 8-bit components in 16 bits (fix8_16),
// then widen those to 8-bit-in-32-bits (fix8_32), convert those to floats (scaled),
// then finally scale those down from [0.0f, 255.0f] to [0.0f, 1.0f] into fColor.

// get() and clamped() do the opposite, working from [0.0f, 1.0f] floats to [0.0f, 255.0f],
// to 8-bit-in-32-bit, to 8-bit-in-16-bit, back down to 8-bit components.
// clamped() uses vqmovn to clamp while narrowing instead of just narrowing with vmovn.

inline void SkPMFloat::set(SkPMColor c) {
    SkPMColorAssert(c);
    uint8x8_t   fix8    = vdup_n_u32(c);
    uint16x8_t  fix8_16 = vmovl_u8(fix8);
    uint32x4_t  fix8_32 = vmovl_u16(vget_low_u16(fix8_16));
    float32x4_t scaled  = vcvtq_f32_u32(fix8_32);
    vst1q_f32(fColor, vmulq_f32(scaled, vdupq_n_f32(1.0f/255.0f)));
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::get() const {
    SkASSERT(this->isValid());
    float32x4_t scaled  = vmulq_f32(vld1q_f32(fColor), vdupq_n_f32(255.0f));
    uint32x4_t  fix8_32 = vcvtq_u32_f32(scaled);
    uint16x4_t  fix8_16 = vmovn_u32(fix8_32);
    uint8x8_t   fix8    = vmovn_u16(vcombine_u16(fix8_16, vdup_n_u16(0)));
    SkPMColor c = vget_lane_u32(fix8, 0);
    SkPMColorAssert(c);
    return c;
}

inline SkPMColor SkPMFloat::clamped() const {
    float32x4_t scaled  = vmulq_f32(vld1q_f32(fColor), vdupq_n_f32(255.0f));
    uint32x4_t  fix8_32 = vcvtq_u32_f32(scaled);
    uint16x4_t  fix8_16 = vqmovn_u32(fix8_32);
    uint8x8_t   fix8    = vqmovn_u16(vcombine_u16(fix8_16, vdup_n_u16(0)));
    SkPMColor c = vget_lane_u32(fix8, 0);
    SkPMColorAssert(c);
    return c;
}
