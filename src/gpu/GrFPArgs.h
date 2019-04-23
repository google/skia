/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFPArgs_DEFINED
#define GrFPArgs_DEFINED

#include "include/core/SkFilterQuality.h"
#include "include/core/SkMatrix.h"

class GrColorSpaceInfo;
class GrRecordingContext;

struct GrFPArgs {
    GrFPArgs(GrRecordingContext* context,
             const SkMatrix* viewMatrix,
             SkFilterQuality filterQuality,
             const GrColorSpaceInfo* dstColorSpaceInfo)
    : fContext(context)
    , fViewMatrix(viewMatrix)
    , fFilterQuality(filterQuality)
    , fDstColorSpaceInfo(dstColorSpaceInfo) {
        SkASSERT(fContext);
        SkASSERT(fViewMatrix);
    }

    class WithPreLocalMatrix;
    class WithPostLocalMatrix;

    GrRecordingContext* fContext;
    const SkMatrix* fViewMatrix;

    // We track both pre and post local matrix adjustments.  For a given FP:
    //
    //   total_local_matrix = postLocalMatrix x FP_localMatrix x preLocalMatrix
    //
    // Use the helpers above to create pre/post GrFPArgs wrappers.
    //
    const SkMatrix* fPreLocalMatrix  = nullptr;
    const SkMatrix* fPostLocalMatrix = nullptr;

    // Make this SkAlphaType?
    bool fInputColorIsOpaque = false;

    SkFilterQuality fFilterQuality;
    const GrColorSpaceInfo* fDstColorSpaceInfo;
};

class GrFPArgs::WithPreLocalMatrix final : public GrFPArgs {
public:
    WithPreLocalMatrix(const GrFPArgs& args, const SkMatrix& lm) : INHERITED(args) {
        if (!lm.isIdentity()) {
            if (fPreLocalMatrix) {
                fStorage.setConcat(lm, *fPreLocalMatrix);
                fPreLocalMatrix = fStorage.isIdentity() ? nullptr : &fStorage;
            } else {
                fPreLocalMatrix = &lm;
            }
        }
    }

private:
    WithPreLocalMatrix(const WithPreLocalMatrix&) = delete;
    WithPreLocalMatrix& operator=(const WithPreLocalMatrix&) = delete;

    SkMatrix fStorage;

    using INHERITED = GrFPArgs;
};

class GrFPArgs::WithPostLocalMatrix final : public GrFPArgs {
public:
    WithPostLocalMatrix(const GrFPArgs& args, const SkMatrix& lm) : INHERITED(args) {
        if (!lm.isIdentity()) {
            if (fPostLocalMatrix) {
                fStorage.setConcat(*fPostLocalMatrix, lm);
                fPostLocalMatrix = fStorage.isIdentity() ? nullptr : &fStorage;
            } else {
                fPostLocalMatrix = &lm;
            }
        }
    }

private:
    WithPostLocalMatrix(const WithPostLocalMatrix&) = delete;
    WithPostLocalMatrix& operator=(const WithPostLocalMatrix&) = delete;

    SkMatrix fStorage;

    using INHERITED = GrFPArgs;
};

#endif

