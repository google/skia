/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorSpaceXform_DEFINED
#define GrColorSpaceXform_DEFINED

#include "GrColor.h"
#include "SkMatrix44.h"
#include "SkRefCnt.h"

class SkColorSpace;

 /**
  * Represents a color gamut transformation (as a 4x4 color matrix)
  */
class GrColorSpaceXform : public SkRefCnt {
public:
    GrColorSpaceXform(const SkMatrix44& srcToDst);

    static sk_sp<GrColorSpaceXform> Make(const SkColorSpace* src, const SkColorSpace* dst);

    const SkMatrix44& srcToDst() const { return fSrcToDst; }

    /**
     * GrGLSLFragmentProcessor::GenKey() must call this and include the returned value in its
     * computed key.
     */
    static uint32_t XformKey(const GrColorSpaceXform* xform) {
        // Code generation changes if there is an xform, but it otherwise constant
        return SkToBool(xform) ? 1 : 0;
    }

    static bool Equals(const GrColorSpaceXform* a, const GrColorSpaceXform* b);

    GrColor4f apply(const GrColor4f& srcColor);

private:
    SkMatrix44 fSrcToDst;
};

#endif
