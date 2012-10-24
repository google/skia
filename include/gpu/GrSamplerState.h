
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrSamplerState_DEFINED
#define GrSamplerState_DEFINED

#include "GrEffect.h"
#include "GrMatrix.h"
#include "GrTypes.h"

#include "SkShader.h"

class GrSamplerState {
public:

    GrSamplerState()
    : fEffect (NULL) {
        GR_DEBUGCODE(fSavedCoordChangeCnt = 0;)
    }

    ~GrSamplerState() {
        GrSafeUnref(fEffect);
        GrAssert(0 == fSavedCoordChangeCnt);
    }

    bool operator ==(const GrSamplerState& other) const {
        // first handle cases where one or the other has no effect
        if (NULL == fEffect) {
            return NULL == other.fEffect;
        } else if (NULL == other.fEffect) {
            return false;
        }

        if (fEffect->getFactory() != other.fEffect->getFactory()) {
            return false;
        }

        if (!fEffect->isEqual(*other.fEffect)) {
            return false;
        }

        return fMatrix == other.fMatrix && fCoordChangeMatrix == other.fCoordChangeMatrix;
    }

    bool operator !=(const GrSamplerState& s) const { return !(*this == s); }

    GrSamplerState& operator =(const GrSamplerState& other) {
        GrSafeAssign(fEffect, other.fEffect);
        if (NULL != fEffect) {
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
        GR_DEBUGCODE(mutable SkAutoTUnref<GrEffect> fEffect;)

        friend class GrSamplerState;
    };

    /**
     * This gets the current coordinate system change. It is the accumulation of
     * preConcatCoordChange calls since the effect was installed. It is used when then caller
     * wants to temporarily change the source geometry coord system, draw something, and then
     * restore the previous coord system (e.g. temporarily draw in device coords).s
     */
    void saveCoordChange(SavedCoordChange* savedCoordChange) const {
        savedCoordChange->fCoordChangeMatrix = fCoordChangeMatrix;
        GrAssert(NULL == savedCoordChange->fEffect.get());
        GR_DEBUGCODE(GrSafeRef(fEffect);)
        GR_DEBUGCODE(savedCoordChange->fEffect.reset(fEffect);)
        GR_DEBUGCODE(++fSavedCoordChangeCnt);
    }

    /**
     * This balances the saveCoordChange call.
     */
    void restoreCoordChange(const SavedCoordChange& savedCoordChange) {
        fCoordChangeMatrix = savedCoordChange.fCoordChangeMatrix;
        GrAssert(savedCoordChange.fEffect.get() == fEffect);
        GR_DEBUGCODE(--fSavedCoordChangeCnt);
        GR_DEBUGCODE(savedCoordChange.fEffect.reset(NULL);)
    }

    /**
     * Gets the texture matrix. This is will be removed soon and be managed by GrEffect.
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
        GrSafeSetNull(fEffect);
    }

    GrEffect* setEffect(GrEffect* effect) {
        GrAssert(0 == fSavedCoordChangeCnt);
        GrSafeAssign(fEffect, effect);
        fMatrix.reset();
        fCoordChangeMatrix.reset();
        return effect;
    }

    GrEffect* setEffect(GrEffect* effect, const GrMatrix& matrix) {
        GrAssert(0 == fSavedCoordChangeCnt);
        GrSafeAssign(fEffect, effect);
        fMatrix = matrix;
        fCoordChangeMatrix.reset();
        return effect;
    }

    const GrEffect* getEffect() const { return fEffect; }

private:
    GrMatrix            fCoordChangeMatrix;
    GrMatrix            fMatrix; // TODO: remove this, store in GrEffect
    GrEffect*           fEffect;

    GR_DEBUGCODE(mutable int fSavedCoordChangeCnt;)
};

#endif

