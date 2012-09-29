
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

class GrTextureParams {
public:
    GrTextureParams() {
        this->reset();
    }

    GrTextureParams(const GrTextureParams& params) {
        *this = params;
    }

    GrTextureParams& operator =(const GrTextureParams& params) {
        fTileModes[0] = params.fTileModes[0];
        fTileModes[1] = params.fTileModes[1];
        fBilerp = params.fBilerp;
        return *this;
    }

    void reset() {
        this->reset(SkShader::kClamp_TileMode, false);
    }

    void reset(SkShader::TileMode tileXAndY, bool filter) {
        fTileModes[0] = fTileModes[1] = tileXAndY;
        fBilerp = filter;
    }
    void reset(SkShader::TileMode tileModes[2], bool filter) {
        fTileModes[0] = tileModes[0];
        fTileModes[1] = tileModes[1];
        fBilerp = filter;
    }

    void setClampNoFilter() {
        fTileModes[0] = fTileModes[1] = SkShader::kClamp_TileMode;
        fBilerp = false;
    }

    void setClamp() {
        fTileModes[0] = fTileModes[1] = SkShader::kClamp_TileMode;
    }

    void setBilerp(bool bilerp) { fBilerp = bilerp; }

    void setTileModeX(const SkShader::TileMode tm) { fTileModes[0] = tm; }
    void setTileModeY(const SkShader::TileMode tm) { fTileModes[1] = tm; }
    void setTileModeXAndY(const SkShader::TileMode tm) { fTileModes[0] = fTileModes[1] = tm; }

    SkShader::TileMode getTileModeX() const { return fTileModes[0]; }

    SkShader::TileMode getTileModeY() const { return fTileModes[1]; }

    bool isTiled() const {
        return SkShader::kClamp_TileMode != fTileModes[0] ||
               SkShader::kClamp_TileMode != fTileModes[1];
    }

    bool isBilerp() const { return fBilerp; }

private:

    SkShader::TileMode fTileModes[2];
    bool               fBilerp;
};

class GrSamplerState {
public:
    static const bool kBilerpDefault = false;

    static const SkShader::TileMode kTileModeDefault = SkShader::kClamp_TileMode;

    /**
     * Default sampler state is set to clamp, use normal sampling mode, be
     * unfiltered, and use identity matrix.
     */
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
        // memcpy() breaks refcounting
        fTextureParams = s.fTextureParams;
        fMatrix = s.fMatrix;

        GrSafeAssign(fCustomStage, s.fCustomStage);

        return *this;
    }

    const GrMatrix& getMatrix() const { return fMatrix; }

    GrTextureParams* textureParams() { return &fTextureParams; }
    const GrTextureParams& getTextureParams() const { return fTextureParams; }
    /**
     * Access the sampler's matrix. See SampleMode for explanation of
     * relationship between the matrix and sample mode.
     */
    GrMatrix* matrix() { return &fMatrix; }

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

    void reset(SkShader::TileMode tileXAndY,
               bool filter,
               const GrMatrix& matrix) {
        fTextureParams.reset(tileXAndY, filter);
        fMatrix = matrix;
        GrSafeSetNull(fCustomStage);
    }
    void reset(SkShader::TileMode wrapXAndY, bool filter) {
        this->reset(wrapXAndY, filter, GrMatrix::I());
    }
    void reset(const GrMatrix& matrix) {
        this->reset(kTileModeDefault, kBilerpDefault, matrix);
    }
    void reset() {
        this->reset(kTileModeDefault, kBilerpDefault, GrMatrix::I());
    }

    GrCustomStage* setCustomStage(GrCustomStage* stage) {
        GrSafeAssign(fCustomStage, stage);
        return stage;
    }
    const GrCustomStage* getCustomStage() const { return fCustomStage; }

private:
    GrTextureParams     fTextureParams;
    GrMatrix            fMatrix;

    GrCustomStage*      fCustomStage;
};

#endif

