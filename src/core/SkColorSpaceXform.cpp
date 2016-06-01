/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform.h"

bool compute_gamut_xform(SkMatrix44* srcToDst, const SkMatrix44& srcToXYZ,
                         const SkMatrix44& dstToXYZ) {
    if (!dstToXYZ.invert(srcToDst)) {
        return false;
    }

    srcToDst->postConcat(srcToXYZ);
    return true;
}

std::unique_ptr<SkColorSpaceXform> SkColorSpaceXform::New(const sk_sp<SkColorSpace>& srcSpace,
                                                          const sk_sp<SkColorSpace>& dstSpace) {
    if (!srcSpace || !dstSpace) {
        return nullptr;
    }

    if (as_CSB(srcSpace)->gammas()->isValues() && as_CSB(dstSpace)->gammas()->isValues()) {
        SkMatrix44 srcToDst(SkMatrix44::kUninitialized_Constructor);
        if (!compute_gamut_xform(&srcToDst, srcSpace->xyz(), dstSpace->xyz())) {
            return nullptr;
        }

        float srcGammas[3];
        float dstGammas[3];
        srcGammas[0] = as_CSB(srcSpace)->gammas()->fRed.fValue;
        srcGammas[1] = as_CSB(srcSpace)->gammas()->fGreen.fValue;
        srcGammas[2] = as_CSB(srcSpace)->gammas()->fBlue.fValue;
        dstGammas[0] = 1.0f / as_CSB(dstSpace)->gammas()->fRed.fValue;
        dstGammas[1] = 1.0f / as_CSB(dstSpace)->gammas()->fGreen.fValue;
        dstGammas[2] = 1.0f / as_CSB(dstSpace)->gammas()->fBlue.fValue;

        return std::unique_ptr<SkColorSpaceXform>(
                new SkGammaByValueXform(srcGammas, srcToDst, dstGammas));
    }

    // Unimplemeted
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkGammaByValueXform::SkGammaByValueXform(float srcGammas[3], const SkMatrix44& srcToDst,
                                         float dstGammas[3])
    : fSrcToDst(srcToDst)
{
    memcpy(fSrcGammas, srcGammas, 3 * sizeof(float));
    memcpy(fDstGammas, dstGammas, 3 * sizeof(float));
}

static uint8_t clamp_float_to_byte(float v) {
    v = v * 255.0f;
    if (v > 255.0f) {
        return 255;
    } else if (v <= 0.0f) {
        return 0;
    } else {
        return (uint8_t) (v + 0.5f);
    }
}

void SkGammaByValueXform::xform_RGBA_8888(uint32_t* dst, const uint32_t* src, uint32_t len) const {
    while (len-- > 0) {
        float srcFloats[3];
        srcFloats[0] = ((*src >>  0) & 0xFF) * (1.0f / 255.0f);
        srcFloats[1] = ((*src >>  8) & 0xFF) * (1.0f / 255.0f);
        srcFloats[2] = ((*src >> 16) & 0xFF) * (1.0f / 255.0f);

        // Convert to linear.
        srcFloats[0] = pow(srcFloats[0], fSrcGammas[0]);
        srcFloats[1] = pow(srcFloats[1], fSrcGammas[1]);
        srcFloats[2] = pow(srcFloats[2], fSrcGammas[2]);

        // Convert to dst gamut.
        float dstFloats[3];
        dstFloats[0] = srcFloats[0] * fSrcToDst.getFloat(0, 0) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 0) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 0) + fSrcToDst.getFloat(3, 0);
        dstFloats[1] = srcFloats[0] * fSrcToDst.getFloat(0, 1) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 1) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 1) + fSrcToDst.getFloat(3, 1);
        dstFloats[2] = srcFloats[0] * fSrcToDst.getFloat(0, 2) +
                       srcFloats[1] * fSrcToDst.getFloat(1, 2) +
                       srcFloats[2] * fSrcToDst.getFloat(2, 2) + fSrcToDst.getFloat(3, 2);

        // Convert to dst gamma.
        dstFloats[0] = pow(dstFloats[0], fDstGammas[0]);
        dstFloats[1] = pow(dstFloats[1], fDstGammas[1]);
        dstFloats[2] = pow(dstFloats[2], fDstGammas[2]);

        *dst = SkPackARGB32NoCheck(((*src >> 24) & 0xFF),
                                   clamp_float_to_byte(dstFloats[0]),
                                   clamp_float_to_byte(dstFloats[1]),
                                   clamp_float_to_byte(dstFloats[2]));

        dst++;
        src++;
    }
}
