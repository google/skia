/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrEllipseEdgeEffect_DEFINED
#define GrEllipseEdgeEffect_DEFINED

#include "GrEffect.h"

class GrGLEllipseEdgeEffect;

/**
 * The output of this effect is a modulation of the input color and coverage for an axis-aligned
 * ellipse, specified as center_x, center_y, x_radius, x_radius/y_radius in window space (y-down).
 */

class GrEllipseEdgeEffect : public GrEffect {
public:
    static GrEffectRef* Create(bool stroke) {
        // we go through this so we only have one copy of each effect (stroked/filled)
        static SkAutoTUnref<GrEffectRef> gEllipseStrokeEdgeEffectRef(
                        CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(GrEllipseEdgeEffect, (true)))));
        static SkAutoTUnref<GrEffectRef> gEllipseFillEdgeEffectRef(
                        CreateEffectRef(AutoEffectUnref(SkNEW_ARGS(GrEllipseEdgeEffect, (false)))));

        if (stroke) {
            gEllipseStrokeEdgeEffectRef.get()->ref();
            return gEllipseStrokeEdgeEffectRef;
        } else {
            gEllipseFillEdgeEffectRef.get()->ref();
            return gEllipseFillEdgeEffectRef;
        }
    }

    virtual ~GrEllipseEdgeEffect() {}

    static const char* Name() { return "EllipseEdge"; }

    virtual void getConstantColorComponents(GrColor* color, uint32_t* validFlags) const SK_OVERRIDE;

    typedef GrGLEllipseEdgeEffect GLEffect;

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE;

    inline bool isStroked() const { return fStroke; }

private:
    GrEllipseEdgeEffect(bool stroke);

    virtual bool onIsEqual(const GrEffect&) const SK_OVERRIDE {
        return true;
    }

    bool fStroke;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

#endif
