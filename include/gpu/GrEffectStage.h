
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
    explicit GrEffectStage(const GrEffect* effect, int attrIndex0 = -1, int attrIndex1 = -1)
    : fEffect(SkRef(effect)) {
        fCoordChangeMatrixSet = false;
        fVertexAttribIndices[0] = attrIndex0;
        fVertexAttribIndices[1] = attrIndex1;
    }

    GrEffectStage(const GrEffectStage& other) {
        *this = other;
    }

    GrEffectStage& operator= (const GrEffectStage& other) {
        fCoordChangeMatrixSet = other.fCoordChangeMatrixSet;
        if (other.fCoordChangeMatrixSet) {
            fCoordChangeMatrix = other.fCoordChangeMatrix;
        }
        fEffect.reset(SkRef(other.fEffect.get()));
        memcpy(fVertexAttribIndices, other.fVertexAttribIndices, sizeof(fVertexAttribIndices));
        return *this;
    }
    
    static bool AreCompatible(const GrEffectStage& a, const GrEffectStage& b,
                              bool usingExplicitLocalCoords) {
        SkASSERT(NULL != a.fEffect.get());
        SkASSERT(NULL != b.fEffect.get());

        if (!a.getEffect()->isEqual(*b.getEffect())) {
            return false;
        }

        // We always track the coord change matrix, but it has no effect when explicit local coords
        // are used.
        if (usingExplicitLocalCoords) {
            return true;
        }

        if (a.fCoordChangeMatrixSet != b.fCoordChangeMatrixSet) {
            return false;
        }

        if (!a.fCoordChangeMatrixSet) {
            return true;
        }

        return a.fCoordChangeMatrix == b.fCoordChangeMatrix;
    }

    /**
     * This is called when the coordinate system in which the geometry is specified will change.
     *
     * @param matrix    The transformation from the old coord system in which geometry is specified
     *                  to the new one from which it will actually be drawn.
     */
    void localCoordChange(const SkMatrix& matrix) {
        if (fCoordChangeMatrixSet) {
            fCoordChangeMatrix.preConcat(matrix);
        } else {
            fCoordChangeMatrixSet = true;
            fCoordChangeMatrix = matrix;
        }
    }

    class SavedCoordChange {
    private:
        bool fCoordChangeMatrixSet;
        SkMatrix fCoordChangeMatrix;
        SkDEBUGCODE(mutable SkAutoTUnref<const GrEffect> fEffect;)

        friend class GrEffectStage;
    };

    /**
     * This gets the current coordinate system change. It is the accumulation of
     * localCoordChange calls since the effect was installed. It is used when then caller
     * wants to temporarily change the source geometry coord system, draw something, and then
     * restore the previous coord system (e.g. temporarily draw in device coords).
     */
    void saveCoordChange(SavedCoordChange* savedCoordChange) const {
        savedCoordChange->fCoordChangeMatrixSet = fCoordChangeMatrixSet;
        if (fCoordChangeMatrixSet) {
            savedCoordChange->fCoordChangeMatrix = fCoordChangeMatrix;
        }
        SkASSERT(NULL == savedCoordChange->fEffect.get());
        SkDEBUGCODE(SkRef(fEffect.get());)
        SkDEBUGCODE(savedCoordChange->fEffect.reset(fEffect.get());)
    }

    /**
     * This balances the saveCoordChange call.
     */
    void restoreCoordChange(const SavedCoordChange& savedCoordChange) {
        fCoordChangeMatrixSet = savedCoordChange.fCoordChangeMatrixSet;
        if (fCoordChangeMatrixSet) {
            fCoordChangeMatrix = savedCoordChange.fCoordChangeMatrix;
        }
        SkASSERT(savedCoordChange.fEffect.get() == fEffect);
        SkDEBUGCODE(savedCoordChange.fEffect.reset(NULL);)
    }

    /**
     * Gets the matrix representing all changes of coordinate system since the GrEffect was
     * installed in the stage.
     */
    const SkMatrix& getCoordChangeMatrix() const {
        if (fCoordChangeMatrixSet) {
            return fCoordChangeMatrix;
        } else {
            return SkMatrix::I();
        }
    }

    const GrEffect* getEffect() const { return fEffect.get(); }

    const int* getVertexAttribIndices() const { return fVertexAttribIndices; }
    int getVertexAttribIndexCount() const { return fEffect->numVertexAttribs(); }

private:
    bool                                fCoordChangeMatrixSet;
    SkMatrix                            fCoordChangeMatrix;
    SkAutoTUnref<const GrEffect>        fEffect;
    int                                 fVertexAttribIndices[2];
};

#endif
