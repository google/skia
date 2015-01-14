/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArithmeticMode_gpu_DEFINED
#define SkArithmeticMode_gpu_DEFINED

#if SK_SUPPORT_GPU

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrTextureAccess.h"

class GrInvariantOutput;
class GrTexture;

///////////////////////////////////////////////////////////////////////////////
// Fragment Processor
///////////////////////////////////////////////////////////////////////////////

class GrGLArtithmeticFP;

class GrArithmeticFP : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(float k1, float k2, float k3, float k4, bool enforcePMColor,
                               GrTexture* background) {
        return SkNEW_ARGS(GrArithmeticFP, (k1, k2, k3, k4, enforcePMColor, background));
    }

    ~GrArithmeticFP() SK_OVERRIDE {};

    const char* name() const SK_OVERRIDE { return "Arithmetic"; }

    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    GrGLFragmentProcessor* createGLInstance() const SK_OVERRIDE;

    GrTexture* backgroundTexture() const { return fBackgroundAccess.getTexture(); }

    float k1() const { return fK1; }
    float k2() const { return fK2; }
    float k3() const { return fK3; }
    float k4() const { return fK4; }
    bool enforcePMColor() const { return fEnforcePMColor; }

private:
    bool onIsEqual(const GrFragmentProcessor&) const SK_OVERRIDE;

    void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE;

    GrArithmeticFP(float k1, float k2, float k3, float k4, bool enforcePMColor,
                   GrTexture* background);

    float                       fK1, fK2, fK3, fK4;
    bool                        fEnforcePMColor;
    GrCoordTransform            fBackgroundTransform;
    GrTextureAccess             fBackgroundAccess;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrFragmentProcessor INHERITED;
};

#endif
#endif
