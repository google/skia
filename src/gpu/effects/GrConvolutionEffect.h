/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConvolutionEffect_DEFINED
#define GrConvolutionEffect_DEFINED

#include "Gr1DKernelEffect.h"

class GrGLConvolutionEffect;

/**
 * A convolution effect. The kernel is specified as an array of 2 * half-width
 * + 1 weights. Each texel is multiplied by it's weight and summed to determine
 * the output color. The output color is modulated by the input color.
 */
class GrConvolutionEffect : public Gr1DKernelEffect {

public:

    /// Convolve with an arbitrary user-specified kernel
    static GrEffectRef* Create(GrTexture* tex,
                               Direction dir,
                               int halfWidth,
                               const float* kernel,
                               bool useBounds,
                               float bounds[2]) {
        AutoEffectUnref effect(SkNEW_ARGS(GrConvolutionEffect, (tex,
                                                                dir,
                                                                halfWidth,
                                                                kernel,
                                                                useBounds,
                                                                bounds)));
        return CreateEffectRef(effect);
    }

    /// Convolve with a Gaussian kernel
    static GrEffectRef* CreateGaussian(GrTexture* tex,
                                       Direction dir,
                                       int halfWidth,
                                       float gaussianSigma,
                                       bool useBounds,
                                       float bounds[2]) {
        AutoEffectUnref effect(SkNEW_ARGS(GrConvolutionEffect, (tex,
                                                                dir,
                                                                halfWidth,
                                                                gaussianSigma,
                                                                useBounds,
                                                                bounds)));
        return CreateEffectRef(effect);
    }

    virtual ~GrConvolutionEffect();

    const float* kernel() const { return fKernel; }

    const float* bounds() const { return fBounds; }
    bool useBounds() const { return fUseBounds; }

    static const char* Name() { return "Convolution"; }

    typedef GrGLConvolutionEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

    virtual void getConstantColorComponents(GrColor*, uint32_t* validFlags) const {
        // If the texture was opaque we could know that the output color if we knew the sum of the
        // kernel values.
        *validFlags = 0;
    }

    enum {
        // This was decided based on the min allowed value for the max texture
        // samples per fragment program run in DX9SM2 (32). A sigma param of 4.0
        // on a blur filter gives a kernel width of 25 while a sigma of 5.0
        // would exceed a 32 wide kernel.
        kMaxKernelRadius = 12,
        // With a C++11 we could have a constexpr version of WidthFromRadius()
        // and not have to duplicate this calculation.
        kMaxKernelWidth = 2 * kMaxKernelRadius + 1,
    };

protected:

    float fKernel[kMaxKernelWidth];
    bool fUseBounds;
    float fBounds[2];

private:
    GrConvolutionEffect(GrTexture*, Direction,
                        int halfWidth,
                        const float* kernel,
                        bool useBounds,
                        float bounds[2]);

    /// Convolve with a Gaussian kernel
    GrConvolutionEffect(GrTexture*, Direction,
                        int halfWidth,
                        float gaussianSigma,
                        bool useBounds,
                        float bounds[2]);

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE;

    GR_DECLARE_EFFECT_TEST;

    typedef Gr1DKernelEffect INHERITED;
};

#endif
