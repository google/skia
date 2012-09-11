/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorTableEffect_DEFINED
#define GrColorTableEffect_DEFINED

#include "GrSingleTextureEffect.h"
#include "GrTexture.h"

class GrGLColorTableEffect;

/**
 * LUT-based color transformation effect. This class implements the Gr
 * counterpart to the SkTable_ColorFilter effect. A 256 * 4 (single-channel)
 * LUT is used to transform the input colors of the image.
 */
class GrColorTableEffect : public GrCustomStage {
public:

    GrColorTableEffect(GrTexture* texture);
    virtual ~GrColorTableEffect();

    static const char* Name() { return "ColorTable"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    virtual int numTextures() const SK_OVERRIDE { return 1; }
    virtual const GrTextureAccess& textureAccess(int index) const SK_OVERRIDE;

    typedef GrGLColorTableEffect GLProgramStage;

private:
    GR_DECLARE_CUSTOM_STAGE_TEST;

    GrTextureAccess fTextureAccess;

    typedef GrCustomStage INHERITED;
};
#endif
