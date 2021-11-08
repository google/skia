/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFPArgs_DEFINED
#define GrFPArgs_DEFINED

#include "include/core/SkMatrix.h"

class GrColorInfo;
class GrRecordingContext;
class SkMatrixProvider;

struct GrFPArgs {
    GrFPArgs(GrRecordingContext* context,
             const SkMatrixProvider& matrixProvider,
             const GrColorInfo* dstColorInfo)
            : fContext(context)
            , fMatrixProvider(matrixProvider)
            , fDstColorInfo(dstColorInfo) {
        SkASSERT(fContext);
    }

    class WithPreLocalMatrix;

    GrFPArgs withNewMatrixProvider(const SkMatrixProvider& provider) const {
        GrFPArgs newArgs(fContext, provider, fDstColorInfo);
        newArgs.fPreLocalMatrix = fPreLocalMatrix;
        return newArgs;
    }

    GrRecordingContext* fContext;
    const SkMatrixProvider& fMatrixProvider;

    const SkMatrix* fPreLocalMatrix  = nullptr;

    const GrColorInfo* fDstColorInfo;
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

#endif
