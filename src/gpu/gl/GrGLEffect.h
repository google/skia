/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLEffect_DEFINED
#define GrGLEffect_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrGLProgramEffects.h"
#include "GrGLShaderBuilder.h"
#include "GrGLShaderVar.h"
#include "GrGLSL.h"

/** @file
    This file contains specializations for OpenGL of the shader stages declared in
    include/gpu/GrEffect.h. Objects of type GrGLEffect are responsible for emitting the
    GLSL code that implements a GrEffect and for uploading uniforms at draw time. If they don't
    always emit the same GLSL code, they must have a function:
        static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&)
    that is used to implement a program cache. When two GrEffects produce the same key this means
    that their GrGLEffects would emit the same GLSL code.

    The GrGLEffect subclass must also have a constructor of the form:
        EffectSubclass::EffectSubclass(const GrBackendEffectFactory&, const GrDrawEffect&)
    The effect held by the GrDrawEffect is guaranteed to be of the type that generated the
    GrGLEffect subclass instance.

    These objects are created by the factory object returned by the GrEffect::getFactory().
*/

class GrDrawEffect;
class GrGLTexture;
class GrGLVertexEffect;

class GrGLEffect {

public:
    typedef GrBackendEffectFactory::EffectKey EffectKey;
    typedef GrGLProgramEffects::TransformedCoordsArray TransformedCoordsArray;
    typedef GrGLProgramEffects::TextureSampler TextureSampler;
    typedef GrGLProgramEffects::TextureSamplerArray TextureSamplerArray;

    enum {
        kNoEffectKey = GrBackendEffectFactory::kNoEffectKey,
        // the number of bits in EffectKey available to GenKey
        kEffectKeyBits = GrBackendEffectFactory::kEffectKeyBits,
    };

    GrGLEffect(const GrBackendEffectFactory& factory)
        : fFactory(factory)
        , fIsVertexEffect(false) {
    }

    virtual ~GrGLEffect() {}

    /** Called when the program stage should insert its code into the shaders. The code in each
        shader will be in its own block ({}) and so locally scoped names will not collide across
        stages.

        @param builder      Interface used to emit code in the shaders.
        @param drawEffect   A wrapper on the effect that generated this program stage.
        @param key          The key that was computed by GenKey() from the generating GrEffect.
                            Only the bits indicated by GrBackendEffectFactory::kEffectKeyBits are
                            guaranteed to match the value produced by GenKey();
        @param outputColor  A predefined vec4 in the FS in which the stage should place its output
                            color (or coverage).
        @param inputColor   A vec4 that holds the input color to the stage in the FS. This may be
                            NULL in which case the implied input is solid white (all ones).
                            TODO: Better system for communicating optimization info (e.g. input
                            color is solid white, trans black, known to be opaque, etc.) that allows
                            the effect to communicate back similar known info about its output.
        @param samplers     One entry for each GrTextureAccess of the GrEffect that generated the
                            GrGLEffect. These can be passed to the builder to emit texture
                            reads in the generated code.
        */
    virtual void emitCode(GrGLShaderBuilder* builder,
                          const GrDrawEffect& drawEffect,
                          EffectKey key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray& coords,
                          const TextureSamplerArray& samplers) = 0;

    /** A GrGLEffect instance can be reused with any GrEffect that produces the same stage
        key; this function reads data from a stage and uploads any uniform variables required
        by the shaders created in emitCode(). The GrEffect installed in the GrEffectStage is
        guaranteed to be of the same type that created this GrGLEffect and to have an identical
        EffectKey as the one that created this GrGLEffect. Effects that use local coords have
        to consider whether the GrEffectStage's coord change matrix should be used. When explicit
        local coordinates are used it can be ignored. */
    virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) {}

    const char* name() const { return fFactory.name(); }

    static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&) { return 0; }

    /** Used by the system when generating shader code, to see if this effect can be downcasted to
        the internal GrGLVertexEffect type */
    bool isVertexEffect() const { return fIsVertexEffect; }

protected:
    const GrBackendEffectFactory& fFactory;

private:
    friend class GrGLVertexEffect; // to set fIsVertexEffect

    bool fIsVertexEffect;
};

#endif
