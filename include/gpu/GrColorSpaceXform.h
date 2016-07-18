/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorSpaceXform_DEFINED
#define GrColorSpaceXform_DEFINED

#include "SkMatrix44.h"
#include "SkRefCnt.h"

class SkColorSpace;

 /**
  * Represents a color gamut transformation (as a 4x4 color matrix)
  */
class GrColorSpaceXform : public SkRefCnt {
public:
    GrColorSpaceXform(const SkMatrix44& srcToDst) : fSrcToDst(srcToDst) {}

    static sk_sp<GrColorSpaceXform> Make(SkColorSpace* src, SkColorSpace* dst);

    const SkMatrix44& srcToDst() { return fSrcToDst; }

private:
    SkMatrix44 fSrcToDst;
};

#endif
