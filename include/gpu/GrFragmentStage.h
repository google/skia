/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFragmentStage_DEFINED
#define GrFragmentStage_DEFINED

#include "GrFragmentProcessor.h"
#include "SkMatrix.h"

/**
 * Wraps a GrFragmentProcessor. It also contains a coord change matrix. This matrix should be
 * concat'ed with all the processor's coord transforms that apply to local coords, unless
 * explicit local coords are provided with the draw.
 */
class GrFragmentStage {
public:
    explicit GrFragmentStage(const GrFragmentProcessor* proc)
    : fProc(SkRef(proc)) {
        fCoordChangeMatrixSet = false;
    }

    GrFragmentStage(const GrFragmentStage& other) {
        fCoordChangeMatrixSet = other.fCoordChangeMatrixSet;
        if (other.fCoordChangeMatrixSet) {
            fCoordChangeMatrix = other.fCoordChangeMatrix;
        }
        fProc.reset(SkRef(other.fProc.get()));
    }

    static bool AreCompatible(const GrFragmentStage& a, const GrFragmentStage& b,
                              bool usingExplicitLocalCoords) {
        SkASSERT(a.fProc.get());
        SkASSERT(b.fProc.get());

        if (!a.getProcessor()->isEqual(*b.getProcessor())) {
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
    public:
        SkDEBUGCODE(SavedCoordChange() : fEffectUniqueID(SK_InvalidUniqueID) {})
    private:
        bool fCoordChangeMatrixSet;
        SkMatrix fCoordChangeMatrix;
        SkDEBUGCODE(mutable uint32_t fEffectUniqueID;)

        friend class GrFragmentStage;
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
        SkASSERT(SK_InvalidUniqueID == savedCoordChange->fEffectUniqueID);
        SkDEBUGCODE(savedCoordChange->fEffectUniqueID = fProc->getUniqueID();)
    }

    /**
     * This balances the saveCoordChange call.
     */
    void restoreCoordChange(const SavedCoordChange& savedCoordChange) {
        fCoordChangeMatrixSet = savedCoordChange.fCoordChangeMatrixSet;
        if (fCoordChangeMatrixSet) {
            fCoordChangeMatrix = savedCoordChange.fCoordChangeMatrix;
        }
        SkASSERT(savedCoordChange.fEffectUniqueID == fProc->getUniqueID());
        SkDEBUGCODE(savedCoordChange.fEffectUniqueID = SK_InvalidUniqueID);
    }

    /**
     * Gets the matrix representing all changes of coordinate system since the GrProcessor was
     * installed in the stage.
     */
    const SkMatrix& getCoordChangeMatrix() const {
        if (fCoordChangeMatrixSet) {
            return fCoordChangeMatrix;
        } else {
            return SkMatrix::I();
        }
    }

    const GrFragmentProcessor* getProcessor() const { return fProc.get(); }

protected:
    bool                                    fCoordChangeMatrixSet;
    SkMatrix                                fCoordChangeMatrix;
    SkAutoTUnref<const GrFragmentProcessor> fProc;
};

#endif
