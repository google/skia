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
#include "SkTLazy.h"

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

    GrContext* fContext;
    const SkMatrix* fViewMatrix;

    SkFilterQuality fFilterQuality;
    const GrColorSpaceInfo* fDstColorSpaceInfo;

    // We track both pre and post local matrix adjustments.  For a given FP:
    //
    //   total_local_matrix = postLocalMatrix x FP_localMatrix x preLocalMatrix
    //
    // Use the helpers below to compute total local matrices, and create
    // pre/post GrFPArgs wrappers.

    SkTCopyOnFirstWrite<SkMatrix> totalLocalMatrix(const SkMatrix& lm) const {
        SkTCopyOnFirstWrite<SkMatrix> tlm(lm);
        if (fPreLocalMatrix) {
            tlm.writable()->preConcat(*fPreLocalMatrix);
        }
        if (fPostLocalMatrix) {
            tlm.writable()->postConcat(*fPostLocalMatrix);
        }
        return tlm;
    }

    class WithMatrixStorage;

    WithMatrixStorage makeWithPreLocalMatrix(const SkMatrix&) const;
    WithMatrixStorage makeWithPostLocalMatrix(const SkMatrix&) const;

private:
    const SkMatrix* fPreLocalMatrix  = nullptr;
    const SkMatrix* fPostLocalMatrix = nullptr;
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

