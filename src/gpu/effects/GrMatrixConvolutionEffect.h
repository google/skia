/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMatrixConvolutionEffect_DEFINED
#define GrMatrixConvolutionEffect_DEFINED

#include "GrSingleTextureEffect.h"

// A little bit less than the minimum # uniforms required by DX9SM2 (32).
// Allows for a 5x5 kernel (or 25x1, for that matter).
#define MAX_KERNEL_SIZE 25

class GrGLMatrixConvolutionEffect;

class GrMatrixConvolutionEffect : public GrSingleTextureEffect {
public:
    /*! \enum TileMode */
    enum TileMode {
        kClamp_TileMode = 0,         /*!< Clamp to the image's edge pixels. */
        kRepeat_TileMode,        /*!< Wrap around to the image's opposite edge. */
        kClampToBlack_TileMode,  /*!< Fill with transparent black. */
        kMax_TileMode = kClampToBlack_TileMode
    };

    typedef GrMatrixConvolutionEffect::TileMode TileMode;
    static GrEffect* Create(GrTexture* texture,
                            const SkIRect& bounds,
                            const SkISize& kernelSize,
                            const SkScalar* kernel,
                            SkScalar gain,
                            SkScalar bias,
                            const SkIPoint& kernelOffset,
                            TileMode tileMode,
                            bool convolveAlpha) {
        return SkNEW_ARGS(GrMatrixConvolutionEffect, (texture,
                                                      bounds,
                                                      kernelSize,
                                                      kernel,
                                                      gain,
                                                      bias,
                                                      kernelOffset,
                                                      tileMode,
                                                      convolveAlpha));
    }
    virtual ~GrMatrixConvolutionEffect();

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        // TODO: Try to do better?
        *validFlags = 0;
    }

    static const char* Name() { return "MatrixConvolution"; }
    const SkIRect& bounds() const { return fBounds; }
    const SkISize& kernelSize() const { return fKernelSize; }
    const float* kernelOffset() const { return fKernelOffset; }
    const float* kernel() const { return fKernel; }
    float gain() const { return fGain; }
    float bias() const { return fBias; }
    TileMode tileMode() const { return fTileMode; }
    bool convolveAlpha() const { return fConvolveAlpha; }

    typedef GrGLMatrixConvolutionEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrMatrixConvolutionEffect(GrTexture*,
                              const SkIRect& bounds,
                              const SkISize& kernelSize,
                              const SkScalar* kernel,
                              SkScalar gain,
                              SkScalar bias,
                              const SkIPoint& kernelOffset,
                              TileMode tileMode,
                              bool convolveAlpha);

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    SkIRect  fBounds;
    SkISize  fKernelSize;
    float   *fKernel;
    float    fGain;
    float    fBias;
    float    fKernelOffset[2];
    TileMode fTileMode;
    bool     fConvolveAlpha;

    GR_DECLARE_EFFECT_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
