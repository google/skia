
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

class GrSamplerState {
public:
    enum Filter {
        /**
         * Read the closest src texel to the sample position
         */
        kNearest_Filter,
        /**
         * Blend between closest 4 src texels to sample position (tent filter)
         */
        kBilinear_Filter,
        /**
         * Average of 4 bilinear filterings spaced +/- 1 texel from sample
         * position in x and y. Intended for averaging 16 texels in a downsample
         * pass. (rasterizing such that texture samples fall exactly halfway
         * between texels in x and y spaced 4 texels apart.) Only supported
         * on shader backends.
         */
        k4x4Downsample_Filter,

        kDefault_Filter = kNearest_Filter
    };

    /**
     * Describes how a texture is sampled when coordinates are outside the
     * texture border
     */
    enum WrapMode {
        kClamp_WrapMode,
        kRepeat_WrapMode,
        kMirror_WrapMode,

        kDefault_WrapMode = kClamp_WrapMode
    };

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
        fWrapX = s.fWrapX;
        fWrapY = s.fWrapY;
        fFilter = s.fFilter;
        fMatrix = s.fMatrix;
        fSwapRAndB = s.fSwapRAndB;
        fTextureDomain = s.fTextureDomain;

        fCustomStage = s.fCustomStage;
        SkSafeRef(fCustomStage);

        return *this;
    }

    WrapMode getWrapX() const { return fWrapX; }
    WrapMode getWrapY() const { return fWrapY; }
    const GrMatrix& getMatrix() const { return fMatrix; }
    const GrRect& getTextureDomain() const { return fTextureDomain; }
    bool hasTextureDomain() const {return SkIntToScalar(0) != fTextureDomain.right();}
    Filter getFilter() const { return fFilter; }
    bool swapsRAndB() const { return fSwapRAndB; }

    void setWrapX(WrapMode mode) { fWrapX = mode; }
    void setWrapY(WrapMode mode) { fWrapY = mode; }
    
    /**
     * Access the sampler's matrix. See SampleMode for explanation of
     * relationship between the matrix and sample mode.
     */
    GrMatrix* matrix() { return &fMatrix; }

    /**
     * Sets the sampler's texture coordinate domain to a 
     * custom rectangle, rather than the default (0,1).
     * This option is currently only supported with kClamp_WrapMode
     */
    void setTextureDomain(const GrRect& textureDomain) { fTextureDomain = textureDomain; }

    /**
     * Swaps the R and B components when reading from the texture. Has no effect
     * if the texture is alpha only.
     */
    void setRAndBSwap(bool swap) { fSwapRAndB = swap; }

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
     * Sets filtering type.
     * @param filter    type of filtering to apply
     */
    void setFilter(Filter filter) { fFilter = filter; }

    void reset(WrapMode wrapXAndY,
               Filter filter,
               const GrMatrix& matrix) {
        fWrapX = wrapXAndY;
        fWrapY = wrapXAndY;
        fFilter = filter;
        fMatrix = matrix;
        fTextureDomain.setEmpty();
        fSwapRAndB = false;
        GrSafeSetNull(fCustomStage);
    }
    void reset(WrapMode wrapXAndY, Filter filter) {
        this->reset(wrapXAndY, filter, GrMatrix::I());
    }
    void reset(const GrMatrix& matrix) {
        this->reset(kDefault_WrapMode, kDefault_Filter, matrix);
    }
    void reset() {
        this->reset(kDefault_WrapMode, kDefault_Filter, GrMatrix::I());
    }

    GrCustomStage* setCustomStage(GrCustomStage* stage) {
        GrSafeAssign(fCustomStage, stage);
        return stage;
    }
    GrCustomStage* getCustomStage() const { return fCustomStage; }

private:
    WrapMode            fWrapX : 8;
    WrapMode            fWrapY : 8;
    Filter              fFilter : 8;
    bool                fSwapRAndB;
    GrMatrix            fMatrix;
    GrRect              fTextureDomain;

    GrCustomStage*      fCustomStage;
};

#endif

