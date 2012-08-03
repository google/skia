/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureDomainEffect_DEFINED
#define GrTextureDomainEffect_DEFINED

//#include "GrCustomStage.h"
#include "GrSingleTextureEffect.h"
#include "GrRect.h"

class GrGLTextureDomainEffect;

/**
 * Limits a texture's lookup coordinates to a domain.
 */
class GrTextureDomainEffect : public GrSingleTextureEffect {

public:

    GrTextureDomainEffect(GrTexture*, GrRect domain);
    virtual ~GrTextureDomainEffect();

    static const char* Name() { return "TextureDomain"; }

    typedef GrGLTextureDomainEffect GLProgramStage;

    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    const GrRect& domain() const { return fTextureDomain; }

protected:

    GrRect fTextureDomain;

private:
    GR_DECLARE_CUSTOM_STAGE_TEST;

    typedef GrSingleTextureEffect INHERITED;
};

#endif
