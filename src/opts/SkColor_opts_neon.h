#ifndef SkColor_opts_neon_DEFINED
#define SkColor_opts_neon_DEFINED

#include "SkTypes.h"

#include <arm_neon.h>

#define NEON_A (SK_A32_SHIFT / 8)
#define NEON_R (SK_R32_SHIFT / 8)
#define NEON_G (SK_G32_SHIFT / 8)
#define NEON_B (SK_B32_SHIFT / 8)

static inline uint16x8_t SkAlpha255To256_neon8(uint8x8_t alpha) {
    return vaddw_u8(vdupq_n_u16(1), alpha);
}

static inline uint8x8_t SkAlphaMul_neon8(uint8x8_t color, uint16x8_t scale) {
    return vshrn_n_u16(vmovl_u8(color) * scale, 8);
}

static inline uint8x8x4_t SkAlphaMulQ_neon8(uint8x8x4_t color, uint16x8_t scale) {
    uint8x8x4_t ret;

    ret.val[NEON_A] = SkAlphaMul_neon8(color.val[NEON_A], scale);
    ret.val[NEON_R] = SkAlphaMul_neon8(color.val[NEON_R], scale);
    ret.val[NEON_G] = SkAlphaMul_neon8(color.val[NEON_G], scale);
    ret.val[NEON_B] = SkAlphaMul_neon8(color.val[NEON_B], scale);

    return ret;
}

#endif /* #ifndef SkColor_opts_neon_DEFINED */
