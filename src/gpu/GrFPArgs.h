/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFPArgs_DEFINED
#define GrFPArgs_DEFINED

#include "SkFilterQuality.h"
#include "SkMatrix.h"

class GrContext;
class GrColorSpaceInfo;

struct GrFPArgs {
    GrFPArgs(GrContext* context,
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

    class WithMatrixStorage;

    WithMatrixStorage makeWithPreLocalMatrix(const SkMatrix&) const;
    WithMatrixStorage makeWithPostLocalMatrix(const SkMatrix&) const;

    GrContext* fContext;
    const SkMatrix* fViewMatrix;

    // We track both pre and post local matrix adjustments.  For a given FP:
    //
    //   total_local_matrix = postLocalMatrix x FP_localMatrix x preLocalMatrix
    //
    // Use the helpers above to create pre/post GrFPArgs wrappers.
    //
    const SkMatrix* fPreLocalMatrix  = nullptr;
    const SkMatrix* fPostLocalMatrix = nullptr;

    SkFilterQuality fFilterQuality;
    const GrColorSpaceInfo* fDstColorSpaceInfo;
};

class GrFPArgs::WithMatrixStorage final : public GrFPArgs {
private:
    explicit WithMatrixStorage(const GrFPArgs& args) : INHERITED(args) {}

    friend struct GrFPArgs;
    SkMatrix fStorage;

    using INHERITED = GrFPArgs;
};

inline GrFPArgs::WithMatrixStorage GrFPArgs::makeWithPreLocalMatrix(const SkMatrix& lm) const {
    WithMatrixStorage newArgs(*this);

    if (!lm.isIdentity()) {
        if (fPreLocalMatrix) {
            newArgs.fStorage.setConcat(lm, *fPreLocalMatrix);
            newArgs.fPreLocalMatrix = newArgs.fStorage.isIdentity() ? nullptr : &newArgs.fStorage;
        } else {
            newArgs.fPreLocalMatrix = &lm;
        }
    }

    return newArgs;
}

inline GrFPArgs::WithMatrixStorage GrFPArgs::makeWithPostLocalMatrix(const SkMatrix& lm) const {
    WithMatrixStorage newArgs(*this);

    if (!lm.isIdentity()) {
        if (fPostLocalMatrix) {
            newArgs.fStorage.setConcat(*fPostLocalMatrix, lm);
            newArgs.fPostLocalMatrix = newArgs.fStorage.isIdentity() ? nullptr : &newArgs.fStorage;
        } else {
            newArgs.fPostLocalMatrix = &lm;
        }
    }

    return newArgs;
}

#endif

