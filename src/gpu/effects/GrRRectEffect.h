/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRRectEffect_DEFINED
#define GrRRectEffect_DEFINED

#include "GrEffect.h"

#include "SkRRect.h"

class GrGLRRectEffect;

/**
 * An effect that performs anti-aliasing for an SkRRect. It doesn't support all varieties of SkRRect
 * so the caller must check for a NULL return from the Create() method.
 */
class GrRRectEffect : public GrEffect {
public:
    static GrEffectRef* Create(const SkRRect&);

    virtual ~GrRRectEffect();
    static const char* Name() { return "RRect"; }

    const SkRRect& getRRect() const { return fRRect; }

    typedef GrGLRRectEffect GLEffect;

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

private:
    GrRRectEffect(const SkRRect&);

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE;

    SkRRect fRRect;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};


#endif
