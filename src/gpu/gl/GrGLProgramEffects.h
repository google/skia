/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramEffects_DEFINED
#define GrGLProgramEffects_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrTexture.h"
#include "GrTextureAccess.h"
#include "GrGLUniformManager.h"

class GrEffectStage;
class GrGLProgramEffectsBuilder;
class GrGLVertexProgramEffectsBuilder;
class GrGLShaderBuilder;
class GrGLFullShaderBuilder;

/**
 * This class encapsulates an array of GrGLEffects and their supporting data (coord transforms
 * and textures). It is built with GrGLProgramEffectsBuilder, then used to manage the necessary GL
 * state and shader uniforms.
 */
class GrGLProgramEffects {
public:
    typedef GrBackendEffectFactory::EffectKey EffectKey;
    typedef GrGLUniformManager::UniformHandle UniformHandle;

    /**
     * These methods generate different portions of an effect's final key.
     */
    static EffectKey GenAttribKey(const GrDrawEffect&);
    static EffectKey GenTransformKey(const GrDrawEffect&);
    static EffectKey GenTextureKey(const GrDrawEffect&, const GrGLCaps&);

    virtual ~GrGLProgramEffects();

    /**
     * Assigns a texture unit to each sampler. It starts on *texUnitIdx and writes the next
     * available unit to *texUnitIdx when it returns.
     */
    void initSamplers(const GrGLUniformManager&, int* texUnitIdx);

    /**
     * Calls setData() on each effect, and sets their transformation matrices and texture bindings.
     */
    virtual void setData(GrGpuGL*,
                         const GrGLUniformManager&,
                         const GrEffectStage* effectStages[]) = 0;

    /**
     * Passed to GrGLEffects so they can add transformed coordinates to their shader code.
     */
    class TransformedCoords {
    public:
        TransformedCoords(const char* name, GrSLType type)
            : fName(name), fType(type) {
        }

        const char* c_str() const { return fName.c_str(); }
        GrSLType type() const { return fType; }
        const SkString& getName() const { return fName; }

    private:
        SkString fName;
        GrSLType fType;
    };

    typedef SkTArray<TransformedCoords> TransformedCoordsArray;

    /**
     * Passed to GrGLEffects so they can add texture reads to their shader code.
     */
    class TextureSampler {
    public:
        TextureSampler(UniformHandle uniform, const GrTextureAccess& access)
            : fSamplerUniform(uniform)
            , fConfigComponentMask(GrPixelConfigComponentMask(access.getTexture()->config())) {
            SkASSERT(0 != fConfigComponentMask);
            memcpy(fSwizzle, access.getSwizzle(), 5);
        }

        UniformHandle samplerUniform() const { return fSamplerUniform; }
        // bitfield of GrColorComponentFlags present in the texture's config.
        uint32_t configComponentMask() const { return fConfigComponentMask; }
        const char* swizzle() const { return fSwizzle; }

    private:
        UniformHandle fSamplerUniform;
        uint32_t      fConfigComponentMask;
        char          fSwizzle[5];
    };

    typedef SkTArray<TextureSampler> TextureSamplerArray;

protected:
    friend class GrGLProgramEffectsBuilder;

    GrGLProgramEffects(int reserveCount)
        : fGLEffects(reserveCount)
        , fSamplers(reserveCount) {
    }

    /**
     * Helper for setData(). Binds all the textures for an effect.
     */
    void bindTextures(GrGpuGL*, const GrEffectRef&, int effectIdx);

    struct Sampler {
        SkDEBUGCODE(Sampler() : fTextureUnit(-1) {})
        UniformHandle fUniform;
        int           fTextureUnit;
    };

    SkTArray<GrGLEffect*>                  fGLEffects;
    SkTArray<SkSTArray<4, Sampler, true> > fSamplers;
};

/**
 * This is a GrGLProgramEffects implementation that does coord transforms with the vertex shader.
 */
class GrGLVertexProgramEffects : public GrGLProgramEffects {
public:
    virtual void setData(GrGpuGL*,
                         const GrGLUniformManager&,
                         const GrEffectStage* effectStages[]) SK_OVERRIDE;

private:
    friend class GrGLVertexProgramEffectsBuilder;

    GrGLVertexProgramEffects(int reserveCount, bool explicitLocalCoords)
        : INHERITED(reserveCount)
        , fTransforms(reserveCount)
        , fHasExplicitLocalCoords(explicitLocalCoords) {
    }

    /**
     * Helper for setData(). Sets all the transform matrices for an effect.
     */
    void setTransformData(const GrGLUniformManager&, const GrDrawEffect&, int effectIdx);

    struct Transform {
        Transform() { fCurrentValue = SkMatrix::InvalidMatrix(); }
        UniformHandle fHandle;
        GrSLType      fType;
        SkMatrix      fCurrentValue;
    };

    SkTArray<SkSTArray<2, Transform, true> > fTransforms;
    bool                                     fHasExplicitLocalCoords;

    typedef GrGLProgramEffects INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

/**
 * This is an abstract base class for constructing different types of GrGLProgramEffects objects.
 */
class GrGLProgramEffectsBuilder {
public:
    /**
     * Emits the effect's shader code, and stores the necessary uniforms internally.
     */
    virtual void emitEffect(const GrEffectStage&,
                            GrGLProgramEffects::EffectKey,
                            const char* outColor,
                            const char* inColor,
                            int stageIndex) = 0;

protected:
    /**
     * Helper for emitEffect(). Emits uniforms for an effect's texture accesses and appends the
     * necessary data to the TextureSamplerArray* object so effects can add texture lookups.
     */
    static void emitSamplers(GrGLShaderBuilder*,
                             GrGLProgramEffects*,
                             const GrEffectRef&,
                             GrGLProgramEffects::TextureSamplerArray*);
};

/**
 * This class is used to construct a GrGLVertexProgramEffects object.
 */
class GrGLVertexProgramEffectsBuilder : public GrGLProgramEffectsBuilder {
public:
    GrGLVertexProgramEffectsBuilder(GrGLFullShaderBuilder*, int reserveCount);

    virtual void emitEffect(const GrEffectStage&,
                            GrGLProgramEffects::EffectKey,
                            const char* outColor,
                            const char* inColor,
                            int stageIndex) SK_OVERRIDE;

    /**
     * Finalizes the building process and returns the effect array. After this call, the builder
     * becomes invalid.
     */
    GrGLProgramEffects* finish() { return fProgramEffects.detach(); }

private:
    /**
     * Helper for emitEffect(). Emits any attributes an effect might have.
     */
    void emitAttributes(const GrEffectStage&);

    /**
     * Helper for emitEffect(). Emits code to implement an effect's coord transforms in the VS.
     * Varyings are added as an outputs of the VS and inputs to the FS. The varyings may be either a
     * vec2f or vec3f depending upon whether perspective interpolation is required or not. The names
     * of the varyings in the VS and FS as well their types are appended to the
     * TransformedCoordsArray* object, which is in turn passed to the effect's emitCode() function.
     */
    void emitTransforms(const GrEffectRef&,
                        GrGLProgramEffects::EffectKey,
                        GrGLProgramEffects::TransformedCoordsArray*);

    GrGLFullShaderBuilder*                  fBuilder;
    SkAutoTDelete<GrGLVertexProgramEffects> fProgramEffects;

    typedef GrGLProgramEffectsBuilder INHERITED;
};

#endif
