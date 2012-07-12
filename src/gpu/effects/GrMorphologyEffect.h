/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMorphologyEffect_DEFINED
#define GrMorphologyEffect_DEFINED

#include "GrContext.h"
#include "Gr1DKernelEffect.h"

class GrGLMorphologyEffect;

/**
 * Morphology effects. Depending upon the type of morphology, either the
 * component-wise min (Erode_Type) or max (Dilate_Type) of all pixels in the
 * kernel is selected as the new color. The new color is modulated by the input
 * color.
 */
class GrMorphologyEffect : public Gr1DKernelEffect {

public:

    typedef GrContext::MorphologyType MorphologyType;

    GrMorphologyEffect(GrTexture*, Direction, int radius, MorphologyType);
    virtual ~GrMorphologyEffect();

    MorphologyType type() const { return fType; }

    static const char* Name() { return "Morphology"; }

    typedef GrGLMorphologyEffect GLProgramStage;

    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

protected:

    MorphologyType fType;

private:

    typedef Gr1DKernelEffect INHERITED;
};
#endif
