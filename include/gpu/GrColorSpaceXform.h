/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorSpaceXform_DEFINED
#define GrColorSpaceXform_DEFINED

#include "SkImageInfo.h"
#include "SkRefCnt.h"

class SkColorSpace;
class SkMatrix44;

 /**
  * Represents a color gamut transformation (as a 4x4 color matrix)
  */
class GrColorSpaceXform : public SkRefCnt {
public:
    GrColorSpaceXform(const SkMatrix44& srcToDst, SkAlphaType srcAlphaType);

    static sk_sp<GrColorSpaceXform> Make(SkColorSpace* src, SkColorSpace* dst,
                                         SkAlphaType srcAlphaType);

    const float* srcToDst() { return fSrcToDst; }
    SkAlphaType alphaType() const { return fSrcAlphaType; }

    /**
     * GrGLSLFragmentProcessor::GenKey() must call this and include the returned value in its
     * computed key.
     */
    static uint32_t XformKey(GrColorSpaceXform* xform) {
        if (!xform) {
            return 0;
        }
        // Code generation just depends on whether the alpha type is premul or not
        return kPremul_SkAlphaType == xform->fSrcAlphaType ? 1 : 2;
    }

private:
    // We store the column-major form of the srcToDst matrix, for easy uploading to uniforms
    float fSrcToDst[16];

    // Alpha type of the source. If it's premul, we need special handling
    SkAlphaType fSrcAlphaType;
};

#endif
