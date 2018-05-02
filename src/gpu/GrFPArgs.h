/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFPArgs_DEFINED
#define GrFPArgs_DEFINED

#include "SkFilterQuality.h"

class SkMatrix;
class GrContext;
class GrColorSpaceInfo;

struct GrFPArgs {
    GrFPArgs(GrContext* context,
             const SkMatrix* viewMatrix,
             const SkMatrix* localMatrix,
             SkFilterQuality filterQuality,
             const GrColorSpaceInfo* dstColorSpaceInfo)
        : fContext(context)
        , fViewMatrix(viewMatrix)
        , fLocalMatrix(localMatrix)
        , fFilterQuality(filterQuality)
        , fDstColorSpaceInfo(dstColorSpaceInfo) {}

    GrFPArgs(GrContext* context,
             const SkMatrix* viewMatrix,
             SkFilterQuality filterQuality,
             const GrColorSpaceInfo* dstColorSpaceInfo)
    : fContext(context)
    , fViewMatrix(viewMatrix)
    , fLocalMatrix(nullptr)
    , fFilterQuality(filterQuality)
    , fDstColorSpaceInfo(dstColorSpaceInfo) {}

    GrContext* fContext;
    const SkMatrix* fViewMatrix;
    const SkMatrix* fLocalMatrix;
    SkFilterQuality fFilterQuality;
    const GrColorSpaceInfo* fDstColorSpaceInfo;
};

#endif

