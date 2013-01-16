
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
    : fEffectPtr (NULL) {
        GR_DEBUGCODE(fSavedCoordChangeCnt = 0;)
    }

    ~GrEffectStage() {
        GrSafeUnref(fEffectPtr);
        GrAssert(0 == fSavedCoordChangeCnt);
    }

    bool operator ==(const GrEffectStage& other) const {
        // first handle cases where one or the other has no effect
        if (NULL == fEffectPtr) {
            return NULL == other.fEffectPtr;
        } else if (NULL == other.fEffectPtr) {
            return false;
        }

        if (this->getEffect()->getFactory() != other.getEffect()->getFactory()) {
            return false;
        }

        if (!this->getEffect()->isEqual(*other.getEffect())) {
            return false;
        }

        return fCoordChangeMatrix == other.fCoordChangeMatrix;
    }

    bool operator !=(const GrEffectStage& s) const { return !(*this == s); }

    GrEffectStage& operator =(const GrEffectStage& other) {
        GrSafeAssign(fEffectPtr, other.fEffectPtr);
        if (NULL != fEffectPtr) {
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
        GR_DEBUGCODE(mutable SkAutoTUnref<const GrEffectRef> fEffectPtr;)

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
        GrAssert(NULL == savedCoordChange->fEffectPtr.get());
        GR_DEBUGCODE(GrSafeRef(fEffectPtr);)
        GR_DEBUGCODE(savedCoordChange->fEffectPtr.reset(fEffectPtr);)
        GR_DEBUGCODE(++fSavedCoordChangeCnt);
    }

    /**
     * This balances the saveCoordChange call.
     */
    void restoreCoordChange(const SavedCoordChange& savedCoordChange) {
        fCoordChangeMatrix = savedCoordChange.fCoordChangeMatrix;
        GrAssert(savedCoordChange.fEffectPtr.get() == fEffectPtr);
        GR_DEBUGCODE(--fSavedCoordChangeCnt);
        GR_DEBUGCODE(savedCoordChange.fEffectPtr.reset(NULL);)
    }

    /**
     * Gets the matrix representing all changes of coordinate system since the GrEffect was
     * installed in the stage.
     */
    const SkMatrix& getCoordChangeMatrix() const { return fCoordChangeMatrix; }

    void reset() {
        GrSafeSetNull(fEffectPtr);
    }

    const GrEffectRef* setEffect(const GrEffectRef* effectPtr) {
        GrAssert(0 == fSavedCoordChangeCnt);
        GrSafeAssign(fEffectPtr, effectPtr);
        fCoordChangeMatrix.reset();
        return effectPtr;
    }

    // TODO: Push GrEffectRef deeper and make this getter return it rather than GrEffect.
    const GrEffect* getEffect() const {
        if (NULL != fEffectPtr) {
            return fEffectPtr->get();
        } else {
            return NULL;
        }
    }

private:
    SkMatrix            fCoordChangeMatrix;
    const GrEffectRef*  fEffectPtr;

    GR_DEBUGCODE(mutable int fSavedCoordChangeCnt;)
};

#endif

