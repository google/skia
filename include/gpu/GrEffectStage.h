
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
    GrEffectStage() {
        fCoordChangeMatrixSet = false;
        fVertexAttribIndices[0] = -1;
        fVertexAttribIndices[1] = -1;
    }

    explicit GrEffectStage(const GrEffectRef* effectRef, int attrIndex0 = -1, int attrIndex1 = -1)
    : fEffectRef(SkSafeRef(effectRef)) {
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
        fEffectRef.reset(SkSafeRef(other.fEffectRef.get()));
        memcpy(fVertexAttribIndices, other.fVertexAttribIndices, sizeof(fVertexAttribIndices));
        return *this;
    }

    bool operator== (const GrEffectStage& other) const {
        // first handle cases where one or the other has no effect
        if (NULL == fEffectRef.get()) {
            return NULL == other.fEffectRef.get();
        } else if (NULL == other.fEffectRef.get()) {
            return false;
        }

        if (!(*this->getEffect())->isEqual(*other.getEffect())) {
            return false;
        }

        if (fCoordChangeMatrixSet != other.fCoordChangeMatrixSet) {
            return false;
        }

        if (!fCoordChangeMatrixSet) {
            return true;
        }

        return fCoordChangeMatrix == other.fCoordChangeMatrix;
    }

    bool operator!= (const GrEffectStage& s) const { return !(*this == s); }

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
        GR_DEBUGCODE(mutable SkAutoTUnref<const GrEffectRef> fEffectRef;)

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
        GrAssert(NULL == savedCoordChange->fEffectRef.get());
        GR_DEBUGCODE(GrSafeRef(fEffectRef.get());)
        GR_DEBUGCODE(savedCoordChange->fEffectRef.reset(fEffectRef.get());)
    }

    /**
     * This balances the saveCoordChange call.
     */
    void restoreCoordChange(const SavedCoordChange& savedCoordChange) {
        fCoordChangeMatrixSet = savedCoordChange.fCoordChangeMatrixSet;
        if (fCoordChangeMatrixSet) {
            fCoordChangeMatrix = savedCoordChange.fCoordChangeMatrix;
        }
        GrAssert(savedCoordChange.fEffectRef.get() == fEffectRef);
        GR_DEBUGCODE(savedCoordChange.fEffectRef.reset(NULL);)
    }

    /**
     * Used when storing a deferred GrDrawState. The DeferredStage allows resources owned by its
     * GrEffect to be recycled through the cache.
     */
    class DeferredStage {
    public:
        DeferredStage() : fEffect(NULL) {
            SkDEBUGCODE(fInitialized = false;)
        }

        ~DeferredStage() {
            if (NULL != fEffect) {
                fEffect->decDeferredRefCounts();
            }
        }

        void saveFrom(const GrEffectStage& stage) {
            GrAssert(!fInitialized);
            if (NULL != stage.fEffectRef.get()) {
                stage.fEffectRef->get()->incDeferredRefCounts();
                fEffect = stage.fEffectRef->get();
                fCoordChangeMatrixSet = stage.fCoordChangeMatrixSet;
                if (fCoordChangeMatrixSet) {
                    fCoordChangeMatrix = stage.fCoordChangeMatrix;
                }
                fVertexAttribIndices[0] = stage.fVertexAttribIndices[0];
                fVertexAttribIndices[1] = stage.fVertexAttribIndices[1];
            }
            SkDEBUGCODE(fInitialized = true;)
        }

        void restoreTo(GrEffectStage* stage) {
            GrAssert(fInitialized);
            if (NULL != fEffect) {
                stage->fEffectRef.reset(GrEffect::CreateEffectRef(fEffect));
                stage->fCoordChangeMatrixSet = fCoordChangeMatrixSet;
                if (fCoordChangeMatrixSet) {
                    stage->fCoordChangeMatrix = fCoordChangeMatrix;
                }
                stage->fVertexAttribIndices[0] = fVertexAttribIndices[0];
                stage->fVertexAttribIndices[1] = fVertexAttribIndices[1];
            } else {
                stage->fEffectRef.reset(NULL);
            }
        }

        bool isEqual(const GrEffectStage& stage, bool ignoreCoordChange) const {
            if (NULL == stage.fEffectRef.get()) {
                return NULL == fEffect;
            } else if (NULL == fEffect) {
                return false;
            }

            if (fVertexAttribIndices[0] != stage.fVertexAttribIndices[0]
                || fVertexAttribIndices[1] != stage.fVertexAttribIndices[1]) {
                return false;
            }

            if (!(*stage.getEffect())->isEqual(*fEffect)) {
                return false;
            }

            if (ignoreCoordChange) {
                // ignore the coordinate change matrix since there are
                // explicit uv coordinates
                return true;
            }

            if (fCoordChangeMatrixSet != stage.fCoordChangeMatrixSet) {
                return false;
            }

            if (!fCoordChangeMatrixSet) {
                return true;
            }

            return fCoordChangeMatrix == stage.fCoordChangeMatrix;
        }

    private:
        const GrEffect*               fEffect;
        bool                          fCoordChangeMatrixSet;
        SkMatrix                      fCoordChangeMatrix;
        int                           fVertexAttribIndices[2];
        SkDEBUGCODE(bool fInitialized;)
    };

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

    void reset() { fEffectRef.reset(NULL); }

    const GrEffectRef* setEffect(const GrEffectRef* effect, int attr0 = -1, int attr1 = -1) {
        fEffectRef.reset(SkSafeRef(effect));
        fCoordChangeMatrixSet = false;

        fVertexAttribIndices[0] = attr0;
        fVertexAttribIndices[1] = attr1;

        return effect;
    }

    const GrEffectRef* getEffect() const { return fEffectRef.get(); }

    const int* getVertexAttribIndices() const { return fVertexAttribIndices; }
    int getVertexAttribIndexCount() const { return fEffectRef->get()->numVertexAttribs(); }

private:
    bool                                fCoordChangeMatrixSet;
    SkMatrix                            fCoordChangeMatrix;
    SkAutoTUnref<const GrEffectRef>     fEffectRef;
    int                                 fVertexAttribIndices[2];
};

#endif
