/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkBlurImage_opts.h"
#include "SkRect.h"

#include <arm_neon.h>

namespace {

enum BlurDirection {
    kX, kY
};

/**
 * Helper function to spread the components of a 32-bit integer into the
 * lower 8 bits of each 16-bit element of a NEON register.
 */

static inline uint16x4_t expand(uint32_t a) {
    // ( ARGB ) -> ( ARGB ARGB ) -> ( A R G B A R G B )
    uint8x8_t v8 = vreinterpret_u8_u32(vdup_n_u32(a));
    // ( A R G B A R G B ) -> ( 0A 0R 0G 0B 0A 0R 0G 0B ) -> ( 0A 0R 0G 0B )
    return vget_low_u16(vmovl_u8(v8));
}

template<BlurDirection srcDirection, BlurDirection dstDirection>
void SkBoxBlur_NEON(const SkPMColor* src, int srcStride, SkPMColor* dst, int kernelSize,
                    int leftOffset, int rightOffset, int width, int height)
{
    const int rightBorder = SkMin32(rightOffset + 1, width);
    const int srcStrideX = srcDirection == kX ? 1 : srcStride;
    const int dstStrideX = dstDirection == kX ? 1 : height;
    const int srcStrideY = srcDirection == kX ? srcStride : 1;
    const int dstStrideY = dstDirection == kX ? width : 1;
    const uint32x4_t scale = vdupq_n_u32((1 << 24) / kernelSize);
    const uint32x4_t half = vdupq_n_u32(1 << 23);
    for (int y = 0; y < height; ++y) {
        uint32x4_t sum = vdupq_n_u32(0);
        const SkPMColor* p = src;
        for (int i = 0; i < rightBorder; ++i) {
            sum = vaddw_u16(sum, expand(*p));
            p += srcStrideX;
        }

        const SkPMColor* sptr = src;
        SkPMColor* dptr = dst;
        for (int x = 0; x < width; ++x) {
            // ( half+sumA*scale half+sumR*scale half+sumG*scale half+sumB*scale )
            uint32x4_t result = vmlaq_u32(half, sum, scale);

            // Saturated conversion to 16-bit.
            // ( AAAA RRRR GGGG BBBB ) -> ( 0A 0R 0G 0B )
            uint16x4_t result16 = vqshrn_n_u32(result, 16);

            // Saturated conversion to 8-bit.
            // ( 0A 0R 0G 0B ) -> ( 0A 0R 0G 0B 0A 0R 0G 0B ) -> ( A R G B A R G B )
            uint8x8_t result8 = vqshrn_n_u16(vcombine_u16(result16, result16), 8);

            // ( A R G B A R G B ) -> ( ARGB ARGB ) -> ( ARGB )
            // Store low 32 bits to destination.
            vst1_lane_u32(dptr, vreinterpret_u32_u8(result8), 0);

            if (x >= leftOffset) {
                const SkPMColor* l = sptr - leftOffset * srcStrideX;
                sum = vsubw_u16(sum, expand(*l));
            }
            if (x + rightOffset + 1 < width) {
                const SkPMColor* r = sptr + (rightOffset + 1) * srcStrideX;
                sum = vaddw_u16(sum, expand(*r));
            }
            sptr += srcStrideX;
            if (srcDirection == kY) {
                SK_PREFETCH(sptr + (rightOffset + 1) * srcStrideX);
            }
            dptr += dstStrideX;
        }
        src += srcStrideY;
        dst += dstStrideY;
    }
}

} // namespace

bool SkBoxBlurGetPlatformProcs_NEON(SkBoxBlurProc* boxBlurX,
                                    SkBoxBlurProc* boxBlurY,
                                    SkBoxBlurProc* boxBlurXY,
                                    SkBoxBlurProc* boxBlurYX) {
    *boxBlurX = SkBoxBlur_NEON<kX, kX>;
    *boxBlurY = SkBoxBlur_NEON<kY, kY>;
    *boxBlurXY = SkBoxBlur_NEON<kX, kY>;
    *boxBlurYX = SkBoxBlur_NEON<kY, kX>;
    return true;
}
