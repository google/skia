
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrSamplerState_DEFINED
#define GrSamplerState_DEFINED

#include "GrCustomStage.h"
#include "GrMatrix.h"
#include "GrTypes.h"

#include "SkShader.h"

class GrSamplerState {
public:

    GrSamplerState()
    : fCustomStage (NULL) {
        GR_DEBUGCODE(fSavedCoordChangeCnt = 0;)
    }

    ~GrSamplerState() {
        GrSafeUnref(fCustomStage);
        GrAssert(0 == fSavedCoordChangeCnt);
    }

    bool operator ==(const GrSamplerState& other) const {
        // first handle cases where one or the other has no custom stage
        if (NULL == fCustomStage) {
            return NULL == other.fCustomStage;
        } else if (NULL == other.fCustomStage) {
            return false;
        }

        if (fCustomStage->getFactory() != other.fCustomStage->getFactory()) {
            return false;
        }

        if (!fCustomStage->isEqual(*other.fCustomStage)) {
            return false;
        }

        return fMatrix == other.fMatrix && fCoordChangeMatrix == other.fCoordChangeMatrix;
    }

    bool operator !=(const GrSamplerState& s) const { return !(*this == s); }

    GrSamplerState& operator =(const GrSamplerState& other) {
        GrSafeAssign(fCustomStage, other.fCustomStage);
        if (NULL != fCustomStage) {
            fMatrix = other.fMatrix;
            fCoordChangeMatrix = other.fCoordChangeMatrix;
        }
        return *this;
    }

    /**
     * This is called when the coordinate system in which the geometry is specified will change.
     *
     * @param matrix    The transformation from the old coord system to the new one.
     */
    void preConcatCoordChange(const GrMatrix& matrix) { fCoordChangeMatrix.preConcat(matrix); }

    class SavedCoordChange {
    private:
        GrMatrix fCoordChangeMatrix;
        GR_DEBUGCODE(mutable SkAutoTUnref<GrCustomStage> fCustomStage;)

        friend class GrSamplerState;
    };

    /**
     * This gets the current coordinate system change. It is the accumulation of
     * preConcatCoordChange calls since the custom stage was installed. It is used when then caller
     * wants to temporarily change the source geometry coord system, draw something, and then
     * restore the previous coord system (e.g. temporarily draw in device coords).s
     */
    void saveCoordChange(SavedCoordChange* savedCoordChange) const {
        savedCoordChange->fCoordChangeMatrix = fCoordChangeMatrix;
        GrAssert(NULL == savedCoordChange->fCustomStage.get());
        GR_DEBUGCODE(GrSafeRef(fCustomStage);)
        GR_DEBUGCODE(savedCoordChange->fCustomStage.reset(fCustomStage);)
        GR_DEBUGCODE(++fSavedCoordChangeCnt);
    }

    /**
     * This balances the saveCoordChange call.
     */
    void restoreCoordChange(const SavedCoordChange& savedCoordChange) {
        fCoordChangeMatrix = savedCoordChange.fCoordChangeMatrix;
        GrAssert(savedCoordChange.fCustomStage.get() == fCustomStage);
        GR_DEBUGCODE(--fSavedCoordChangeCnt);
        GR_DEBUGCODE(savedCoordChange.fCustomStage.reset(NULL);)
    }

    /**
     * Gets the texture matrix. This is will be removed soon and be managed by GrCustomStage.
     */
    const GrMatrix& getMatrix() const { return fMatrix; }

    /**
     * Gets the matrix to apply at draw time. This is the original texture matrix combined with
     * any coord system changes.
     */
    void getTotalMatrix(GrMatrix* matrix) const {
        *matrix = fMatrix;
        matrix->preConcat(fCoordChangeMatrix);
    }

    void reset() {
        GrSafeSetNull(fCustomStage);
    }

    GrCustomStage* setCustomStage(GrCustomStage* stage) {
        GrAssert(0 == fSavedCoordChangeCnt);
        GrSafeAssign(fCustomStage, stage);
        fMatrix.reset();
        fCoordChangeMatrix.reset();
        return stage;
    }

    GrCustomStage* setCustomStage(GrCustomStage* stage, const GrMatrix& matrix) {
        GrAssert(0 == fSavedCoordChangeCnt);
        GrSafeAssign(fCustomStage, stage);
        fMatrix = matrix;
        fCoordChangeMatrix.reset();
        return stage;
    }

    const GrCustomStage* getCustomStage() const { return fCustomStage; }

private:
    GrMatrix            fCoordChangeMatrix;
    GrMatrix            fMatrix; // TODO: remove this, store in GrCustomStage
    GrCustomStage*      fCustomStage;

    GR_DEBUGCODE(mutable int fSavedCoordChangeCnt;)
};

#endif

