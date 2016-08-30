/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorSpaceXform_DEFINED
#define GrColorSpaceXform_DEFINED

#include "SkRefCnt.h"

class SkColorSpace;
class SkMatrix44;

 /**
  * Represents a color gamut transformation (as a 4x4 color matrix)
  */
class GrColorSpaceXform : public SkRefCnt {
public:
    GrColorSpaceXform(const SkMatrix44& srcToDst);

    static sk_sp<GrColorSpaceXform> Make(SkColorSpace* src, SkColorSpace* dst);

    const float* srcToDst() { return fSrcToDst; }

private:
    // We store the column-major form of the srcToDst matrix, for easy uploading to uniforms
    float fSrcToDst[16];
};

#endif
