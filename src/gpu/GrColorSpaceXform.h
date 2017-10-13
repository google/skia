/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorSpaceXform_DEFINED
#define GrColorSpaceXform_DEFINED

#include "GrColor.h"
#include "SkColorSpace.h"
#include "SkMatrix44.h"
#include "SkRefCnt.h"

 /**
  * Represents a color gamut transformation
  */
class GrColorSpaceXform : public SkRefCnt {
public:
    GrColorSpaceXform(const SkColorSpaceTransferFn&, const SkMatrix44&, uint32_t);

    static sk_sp<GrColorSpaceXform> Make(const SkColorSpace* src,
                                         GrPixelConfig srcConfig,
                                         const SkColorSpace* dst);
    static sk_sp<GrColorSpaceXform> MakeGamutXform(const SkColorSpace* src,
                                                   const SkColorSpace* dst) {
        auto result = Make(src, kUnknown_GrPixelConfig, dst);
        SkASSERT(0 == (result->fFlags & ~kApplyGamutXform_Flag));
        return result;
    }

    const SkColorSpaceTransferFn& transferFn() const { return fSrcTransferFn; }
    const float* transferFnCoeffs() const {
        static_assert(0 == offsetof(SkColorSpaceTransferFn, fG), "TransferFn layout");
        return &fSrcTransferFn.fG;
    }

    const SkMatrix44& gamutXform() const { return fGamutXform; }

    /**
     * GrGLSLFragmentProcessor::GenKey() must call this and include the returned value in its
     * computed key.
     */
    static uint32_t XformKey(const GrColorSpaceXform* xform) {
        // Code generation depends on which steps we apply (as encoded by fFlags)
        return SkToBool(xform) ? xform->fFlags : 0;
    }

    static bool Equals(const GrColorSpaceXform* a, const GrColorSpaceXform* b);

    GrColor4f apply(const GrColor4f& srcColor);

private:
    friend class GrGLSLColorSpaceXformHelper;

    enum Flags {
        kApplyTransferFn_Flag = 0x1,
        kApplyGamutXform_Flag = 0x2,

        // Almost never used. This handles the case where the src data is sRGB pixel config,
        // but the color space has a different transfer function. In that case, we first undo
        // the HW sRGB -> Linear conversion, before applying any other steps.
        kApplyInverseSRGB_Flag = 0x4,
    };

    SkColorSpaceTransferFn fSrcTransferFn;
    SkMatrix44 fGamutXform;
    uint32_t fFlags;
};

#endif
