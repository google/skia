/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrConvolutionEffect_DEFINED
#define GrConvolutionEffect_DEFINED

#include "GrCustomStage.h"
#include "GrSamplerState.h" // for MAX_KENEL_WIDTH, FilterDirection

class GrGLConvolutionEffect;

class GrConvolutionEffect : public GrCustomStage {

public:

    GrConvolutionEffect(GrSamplerState::FilterDirection direction,
                        unsigned int kernelWidth, const float* kernel);
    virtual ~GrConvolutionEffect();

    unsigned int width() const { return fKernelWidth; }
    const float* kernel() const { return fKernel; }
    GrSamplerState::FilterDirection direction() const { return fDirection; }
    
    static const char* Name() { return "Convolution"; }

    typedef GrGLConvolutionEffect GLProgramStage;
    
    virtual const char* name() const SK_OVERRIDE;
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage *) const SK_OVERRIDE;

protected:

    GrSamplerState::FilterDirection fDirection;
    unsigned int fKernelWidth;
    float fKernel[MAX_KERNEL_WIDTH];


private:

    typedef GrCustomStage INHERITED;
};

#endif
