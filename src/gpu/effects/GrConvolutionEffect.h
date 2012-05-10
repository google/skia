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

class GrConvolutionEffect : public GrCustomStage {

public:

    GrConvolutionEffect(GrSamplerState::FilterDirection direction,
                        unsigned int kernelWidth, const float* kernel);
    virtual ~GrConvolutionEffect();

    virtual const char* name() const SK_OVERRIDE;
    virtual GrProgramStageFactory* getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage *) const SK_OVERRIDE;

    unsigned int width() const { return fKernelWidth; }

protected:

    GrSamplerState::FilterDirection fDirection;
    unsigned int fKernelWidth;
    float fKernel[MAX_KERNEL_WIDTH];

    friend class GrGLConvolutionEffect;

private:

    typedef GrCustomStage INHERITED;
};

#endif
