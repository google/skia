/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPendingProcessorStage_DEFINED
#define GrPendingProcessorStage_DEFINED

#include "GrFragmentStage.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrPendingProgramElement.h"
#include "SkMatrix.h"

/**
 * This a baked variant of GrFragmentStage, as recorded in GrOptDrawState.
 */
class GrPendingFragmentStage {
public:
    GrPendingFragmentStage(const GrFragmentStage& stage, bool ignoreMatrix)
    : fProc(stage.getProcessor())
    , fCoordChangeMatrix(ignoreMatrix ? SkMatrix::I() : stage.getCoordChangeMatrix()) {
    }

    GrPendingFragmentStage(const GrPendingFragmentStage& that) { *this = that; }

    GrPendingFragmentStage& operator=(const GrPendingFragmentStage& that) {
        fProc.reset(that.fProc.get());
        fCoordChangeMatrix = that.fCoordChangeMatrix;
        return *this;
    }

    bool operator==(const GrPendingFragmentStage& that) const {
        return this->getProcessor()->isEqual(*that.getProcessor()) &&
               fCoordChangeMatrix == that.fCoordChangeMatrix;
    }

    bool operator!=(const GrPendingFragmentStage& that) const { return !(*this == that); }

    const SkMatrix& getCoordChangeMatrix() const { return fCoordChangeMatrix; }

    /**
     * For a coord transform on the fragment processor, does it or the coord change matrix (if
     * relevant) contain perspective?
     */
    bool isPerspectiveCoordTransform(int matrixIndex) const {
        const GrCoordTransform& coordTransform = this->getProcessor()->coordTransform(matrixIndex);
        uint32_t type = coordTransform.getMatrix().getType();
        if (kLocal_GrCoordSet == coordTransform.sourceCoords()) {
            type |= this->getCoordChangeMatrix().getType();
        }

        return SkToBool(SkMatrix::kPerspective_Mask & type);
    }

    const char* name() const { return fProc->name(); }

    const GrFragmentProcessor* getProcessor() const { return fProc.get(); }

protected:
    GrPendingProgramElement<const GrFragmentProcessor>  fProc;
    SkMatrix                                            fCoordChangeMatrix;
};
#endif
