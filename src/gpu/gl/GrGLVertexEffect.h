/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLVertexEffect_DEFINED
#define GrGLVertexEffect_DEFINED

#include "GrGLEffect.h"

/**
 * If a GL effect needs a GrGLFullShaderBuilder* object to emit vertex code, then it must inherit
 * from this class. Since paths don't have vertices, this class is only meant to be used internally
 * by skia, for special cases.
 */
class GrGLVertexEffect : public GrGLEffect {
public:
    GrGLVertexEffect(const GrBackendEffectFactory& factory)
        : INHERITED(factory) { fIsVertexEffect = true; }

    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    virtual void emitCode(GrGLFullShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray& coords,
                          const TextureSamplerArray& samplers) = 0;

    /**
     * Provide a default override for base class's emitCode() function.
     */
    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray& coords,
                          const TextureSamplerArray& samplers) SK_OVERRIDE {
        GrCrash("GrGLVertexEffect requires GrGLFullShaderBuilder* overload for emitCode().");
    }

private:
    typedef GrGLEffect INHERITED;
};

#endif
