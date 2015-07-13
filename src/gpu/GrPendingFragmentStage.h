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

/**
 * This a baked variant of GrFragmentStage, as recorded in GrOptDrawState.
 */
class GrPendingFragmentStage {
public:
    GrPendingFragmentStage(const GrFragmentStage& stage) : fProc(stage.processor()) {}

    GrPendingFragmentStage(const GrPendingFragmentStage& that) { *this = that; }

    GrPendingFragmentStage& operator=(const GrPendingFragmentStage& that) {
        fProc.reset(that.fProc.get());
        return *this;
    }

    bool operator==(const GrPendingFragmentStage& that) const {
        return this->processor()->isEqual(*that.processor());
    }

    bool operator!=(const GrPendingFragmentStage& that) const { return !(*this == that); }

    /**
     * For a coord transform on the fragment processor, does it or the coord change matrix (if
     * relevant) contain perspective?
     */
    bool isPerspectiveCoordTransform(int matrixIndex) const {
        const GrCoordTransform& coordTransform = this->processor()->coordTransform(matrixIndex);
        uint32_t type = coordTransform.getMatrix().getType();
        return SkToBool(SkMatrix::kPerspective_Mask & type);
    }

    const char* name() const { return fProc->name(); }

    const GrFragmentProcessor* processor() const { return fProc.get(); }

protected:
    GrPendingProgramElement<const GrFragmentProcessor>  fProc;
};
#endif
