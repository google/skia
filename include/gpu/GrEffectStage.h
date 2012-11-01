
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrEffectStage_DEFINED
#define GrEffectStage_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrEffect.h"
#include "SkMatrix.h"
#include "GrTypes.h"

#include "SkShader.h"

class GrEffectStage {
public:

    GrEffectStage()
    : fEffect (NULL) {
        GR_DEBUGCODE(fSavedCoordChangeCnt = 0;)
    }

    ~GrEffectStage() {
        GrSafeUnref(fEffect);
        GrAssert(0 == fSavedCoordChangeCnt);
    }

    bool operator ==(const GrEffectStage& other) const {
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

    bool operator !=(const GrEffectStage& s) const { return !(*this == s); }

    GrEffectStage& operator =(const GrEffectStage& other) {
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
    void preConcatCoordChange(const SkMatrix& matrix) { fCoordChangeMatrix.preConcat(matrix); }

    class SavedCoordChange {
    private:
        SkMatrix fCoordChangeMatrix;
        GR_DEBUGCODE(mutable SkAutoTUnref<const GrEffect> fEffect;)

        friend class GrEffectStage;
    };

    /**
     * This gets the current coordinate system change. It is the accumulation of
     * preConcatCoordChange calls since the effect was installed. It is used when then caller
     * wants to temporarily change the source geometry coord system, draw something, and then
     * restore the previous coord system (e.g. temporarily draw in device coords).
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
    const SkMatrix& getMatrix() const { return fMatrix; }

    /**
     * Gets the matrix to apply at draw time. This is the original texture matrix combined with
     * any coord system changes. This will be removed when the matrix is managed by GrEffect.
     */
    void getTotalMatrix(SkMatrix* matrix) const {
        *matrix = fMatrix;
        matrix->preConcat(fCoordChangeMatrix);
    }

    /**
     * Gets the matrix representing all changes of coordinate system since the GrEffect was
     * installed in the stage.
     */
    const SkMatrix& getCoordChangeMatrix() const { return fCoordChangeMatrix; }

    void reset() {
        GrSafeSetNull(fEffect);
    }

    const GrEffect* setEffect(const GrEffect* effect) {
        GrAssert(0 == fSavedCoordChangeCnt);
        GrSafeAssign(fEffect, effect);
        fMatrix.reset();
        fCoordChangeMatrix.reset();
        return effect;
    }

    const GrEffect* setEffect(const GrEffect* effect, const SkMatrix& matrix) {
        GrAssert(0 == fSavedCoordChangeCnt);
        GrSafeAssign(fEffect, effect);
        fMatrix = matrix;
        fCoordChangeMatrix.reset();
        return effect;
    }

    const GrEffect* getEffect() const { return fEffect; }

private:
    SkMatrix            fCoordChangeMatrix;
    SkMatrix            fMatrix; // TODO: remove this, store in GrEffect
    const GrEffect*     fEffect;

    GR_DEBUGCODE(mutable int fSavedCoordChangeCnt;)
};

#endif

