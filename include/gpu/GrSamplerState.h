
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

#define MAX_KERNEL_WIDTH 25

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
        /**
         * Apply a separable convolution kernel.
         */
        kConvolution_Filter,
        /**
         * Apply a dilate filter (max over a 1D radius).
         */
        kDilate_Filter,
        /**
         * Apply an erode filter (min over a 1D radius).
         */
        kErode_Filter,

        kDefault_Filter = kNearest_Filter
    };

    /**
     * The intepretation of the texture matrix depends on the sample mode. The
     * texture matrix is applied both when the texture coordinates are explicit
     * and  when vertex positions are used as texture  coordinates. In the latter
     * case the texture matrix is applied to the pre-view-matrix position 
     * values.
     *
     * kNormal_SampleMode
     *  The post-matrix texture coordinates are in normalize space with (0,0) at
     *  the top-left and (1,1) at the bottom right.
     * kRadial_SampleMode
     *  The matrix specifies the radial gradient parameters.
     *  (0,0) in the post-matrix space is center of the radial gradient.
     * kRadial2_SampleMode
     *   Matrix transforms to space where first circle is centered at the
     *   origin. The second circle will be centered (x, 0) where x may be 
     *   0 and is provided by setRadial2Params. The post-matrix space is 
     *   normalized such that 1 is the second radius - first radius.
     * kSweepSampleMode
     *  The angle from the origin of texture coordinates in post-matrix space
     *  determines the gradient value.
     */
    enum SampleMode {
        kNormal_SampleMode,     //!< sample color directly
        kRadial_SampleMode,     //!< treat as radial gradient
        kRadial2_SampleMode,    //!< treat as 2-point radial gradient
        kSweep_SampleMode,      //!< treat as sweep gradient

        kDefault_SampleMode = kNormal_SampleMode
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
     * For the filters which perform more than one texture sample (convolution,
     * erode, dilate), this determines the direction in which the texture
     * coordinates will be incremented.
     */
    enum FilterDirection {
        kX_FilterDirection,
        kY_FilterDirection,

        kDefault_FilterDirection = kX_FilterDirection,
    };
    /**
     * Default sampler state is set to clamp, use normal sampling mode, be
     * unfiltered, and use identity matrix.
     */
    GrSamplerState()
    : fRadial2CenterX1()
    , fRadial2Radius0()
    , fRadial2PosRoot()
    , fCustomStage (NULL) {
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
                 fCustomStage->isEqual(s.fCustomStage)));
    }
    bool operator !=(const GrSamplerState& s) const { return !(*this == s); }

    GrSamplerState& operator =(const GrSamplerState s) {
        memcpy(this, &s, sizeof(GrSamplerState));
        return *this;
    }

    WrapMode getWrapX() const { return fWrapX; }
    WrapMode getWrapY() const { return fWrapY; }
    FilterDirection getFilterDirection() const { return fFilterDirection; }
    SampleMode getSampleMode() const { return fSampleMode; }
    const GrMatrix& getMatrix() const { return fMatrix; }
    const GrRect& getTextureDomain() const { return fTextureDomain; }
    bool hasTextureDomain() const {return SkIntToScalar(0) != fTextureDomain.right();}
    Filter getFilter() const { return fFilter; }
    int getKernelWidth() const { return fKernelWidth; }
    const float* getKernel() const { return fKernel; }
    bool swapsRAndB() const { return fSwapRAndB; }

    bool isGradient() const {
        return  kRadial_SampleMode == fSampleMode ||
                kRadial2_SampleMode == fSampleMode ||
                kSweep_SampleMode == fSampleMode;
    }

    void setWrapX(WrapMode mode) { fWrapX = mode; }
    void setWrapY(WrapMode mode) { fWrapY = mode; }
    void setSampleMode(SampleMode mode) { fSampleMode = mode; }
    void setFilterDirection(FilterDirection mode) { fFilterDirection = mode; }
    
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
               FilterDirection direction,
               const GrMatrix& matrix) {
        fWrapX = wrapXAndY;
        fWrapY = wrapXAndY;
        fSampleMode = kDefault_SampleMode;
        fFilter = filter;
        fFilterDirection = direction;
        fMatrix = matrix;
        fTextureDomain.setEmpty();
        fSwapRAndB = false;
        GrSafeSetNull(fCustomStage);
    }
    void reset(WrapMode wrapXAndY, Filter filter, const GrMatrix& matrix) {
        this->reset(wrapXAndY, filter, kDefault_FilterDirection, matrix);
    }
    void reset(WrapMode wrapXAndY,
               Filter filter) {
        this->reset(wrapXAndY, filter, kDefault_FilterDirection, GrMatrix::I());
    }
    void reset(const GrMatrix& matrix) {
        this->reset(kDefault_WrapMode, kDefault_Filter, kDefault_FilterDirection, matrix);
    }
    void reset() {
        this->reset(kDefault_WrapMode, kDefault_Filter, kDefault_FilterDirection, GrMatrix::I());
    }

    GrScalar getRadial2CenterX1() const { return fRadial2CenterX1; }
    GrScalar getRadial2Radius0() const { return fRadial2Radius0; }
    bool     isRadial2PosRoot() const { return SkToBool(fRadial2PosRoot); }
    // do the radial gradient params lead to a linear (rather than quadratic)
    // equation.
    bool radial2IsDegenerate() const { return GR_Scalar1 == fRadial2CenterX1; }

    /**
     * Sets the parameters for kRadial2_SampleMode. The texture
     * matrix must be set so that the first point is at (0,0) and the second
     * point lies on the x-axis. The second radius minus the first is 1 unit.
     * The additional parameters to define the gradient are specified by this
     * function.
     */
    void setRadial2Params(GrScalar centerX1, GrScalar radius0, bool posRoot) {
        fRadial2CenterX1 = centerX1;
        fRadial2Radius0 = radius0;
        fRadial2PosRoot = posRoot;
    }

    void setConvolutionParams(int kernelWidth, const float* kernel) {
        GrAssert(kernelWidth >= 0 && kernelWidth <= MAX_KERNEL_WIDTH);
        fKernelWidth = kernelWidth;
        if (NULL != kernel) {
            memcpy(fKernel, kernel, kernelWidth * sizeof(float));
        }
    }

    void setMorphologyRadius(int radius) {
        GrAssert(radius >= 0 && radius <= MAX_KERNEL_WIDTH);
        fKernelWidth = radius;
    }

    void setCustomStage(GrCustomStage* stage) {
        GrSafeAssign(fCustomStage, stage);
    }
    GrCustomStage* getCustomStage() const { return fCustomStage; }

private:
    WrapMode            fWrapX : 8;
    WrapMode            fWrapY : 8;
    FilterDirection     fFilterDirection : 8;
    SampleMode          fSampleMode : 8;
    Filter              fFilter : 8;
    GrMatrix            fMatrix;
    bool                fSwapRAndB;
    GrRect              fTextureDomain;

    // these are undefined unless fSampleMode == kRadial2_SampleMode
    GrScalar            fRadial2CenterX1;
    GrScalar            fRadial2Radius0;
    SkBool8             fRadial2PosRoot;

    // These are undefined unless fFilter == kConvolution_Filter
    uint8_t             fKernelWidth;
    float               fKernel[MAX_KERNEL_WIDTH];

    GrCustomStage*      fCustomStage;
};

#endif

