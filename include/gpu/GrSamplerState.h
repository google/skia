
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
        memset(this, 0, sizeof(GrSamplerState));
        this->reset();
    }

    ~GrSamplerState() {
        GrSafeUnref(fCustomStage);
    }

    bool operator ==(const GrSamplerState& s) const {
        /* We must be bit-identical as far as the CustomStage;
           there may be multiple CustomStages that will produce
           the same shader code and so are equivalent.
           Can't take the address of fWrapX because it's :8 */
        int bitwiseRegion = (intptr_t) &fCustomStage - (intptr_t) this;
        GrAssert(sizeof(GrSamplerState) ==
                 bitwiseRegion + sizeof(fCustomStage));
        return !memcmp(this, &s, bitwiseRegion) &&
               ((fCustomStage == s.fCustomStage) ||
                (fCustomStage && s.fCustomStage &&
                 (fCustomStage->getFactory() ==
                     s.fCustomStage->getFactory()) &&
                 fCustomStage->isEqual(*s.fCustomStage)));
    }
    bool operator !=(const GrSamplerState& s) const { return !(*this == s); }

    GrSamplerState& operator =(const GrSamplerState& s) {
        fMatrix = s.fMatrix;
        GrSafeAssign(fCustomStage, s.fCustomStage);
        return *this;
    }

    const GrMatrix& getMatrix() const { return fMatrix; }

    /**
     *  Multiplies the current sampler matrix  a matrix
     *
     *  After this call M' = M*m where M is the old matrix, m is the parameter
     *  to this function, and M' is the new matrix. (We consider points to
     *  be column vectors so tex cood vector t is transformed by matrix X as
     *  t' = X*t.)
     *
     *  @param matrix   the matrix used to modify the matrix.
     */
    void preConcatMatrix(const GrMatrix& matrix) { fMatrix.preConcat(matrix); }

    /**
     * Do not call this function. It will be removed soon.
     */
    void setMatrixDeprecated(const GrMatrix& matrix) { fMatrix = matrix; }

    void reset() {
        fMatrix.reset();
        GrSafeSetNull(fCustomStage);
    }

    GrCustomStage* setCustomStage(GrCustomStage* stage) {
        GrSafeAssign(fCustomStage, stage);
        fMatrix.reset();
        return stage;
    }

    GrCustomStage* setCustomStage(GrCustomStage* stage, const GrMatrix& matrix) {
        GrSafeAssign(fCustomStage, stage);
        fMatrix = matrix;
        return stage;
    }

    const GrCustomStage* getCustomStage() const { return fCustomStage; }

private:
    GrMatrix            fMatrix;

    GrCustomStage*      fCustomStage;
};

#endif

