/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramEffects_DEFINED
#define GrGLProgramEffects_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrGLUniformManager.h"
#include "GrTexture.h"
#include "GrTextureAccess.h"

class GrEffect;
class GrEffectStage;
class GrGLVertexProgramEffectsBuilder;
class GrGLShaderBuilder;
class GrGLFullShaderBuilder;
class GrGLFragmentOnlyShaderBuilder;

/**
 * This class encapsulates an array of GrGLEffects and their supporting data (coord transforms
 * and textures). It is built with GrGLProgramEffectsBuilder, then used to manage the necessary GL
 * state and shader uniforms.
 */
class GrGLProgramEffects : public SkRefCnt {
public:
    typedef GrGLUniformManager::UniformHandle UniformHandle;

    /**
     * This class emits some of the code inserted into the shaders for an effect. The code it
     * creates may be dependent on properties of the effect that the effect itself doesn't use
     * in its key (e.g. the pixel format of textures used). So this class inserts a meta-key for
     * every effect using this function. It is also responsible for inserting the effect's class ID
     * which must be different for every GrEffect subclass. It can fail if an effect uses too many
     * textures, attributes, etc for the space allotted in the meta-key.
     */
    static bool GenEffectMetaKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*);

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
        TransformedCoords(const SkString& name, GrSLType type)
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
    /**
     * Helpers for GenEffectMetaKey.
     */
    static uint32_t GenAttribKey(const GrDrawEffect&);
    static uint32_t GenTransformKey(const GrDrawEffect&);
    static uint32_t GenTextureKey(const GrDrawEffect&, const GrGLCaps&);

    GrGLProgramEffects(int reserveCount)
        : fGLEffects(reserveCount)
        , fSamplers(reserveCount) {
    }

    /**
     * Helper for emitEffect() in a subclasses. Emits uniforms for an effect's texture accesses and
     * appends the necessary data to the TextureSamplerArray* object so effects can add texture
     * lookups to their code. This method is only meant to be called during the construction phase.
     */
    void emitSamplers(GrGLShaderBuilder*, const GrEffect*, TextureSamplerArray*);

    /**
     * Helper for setData(). Binds all the textures for an effect.
     */
    void bindTextures(GrGpuGL*, const GrEffect*, int effectIdx);

    struct Sampler {
        SkDEBUGCODE(Sampler() : fTextureUnit(-1) {})
        UniformHandle fUniform;
        int           fTextureUnit;
    };

    SkTArray<GrGLEffect*>                  fGLEffects;
    SkTArray<SkSTArray<4, Sampler, true> > fSamplers;

private:
    typedef SkRefCnt INHERITED;
};

/**
 * This is an abstract base class for constructing different types of GrGLProgramEffects objects.
 */
class GrGLProgramEffectsBuilder {
public:
    virtual ~GrGLProgramEffectsBuilder() { }

    /**
     * Emits the effect's shader code, and stores the necessary uniforms internally.
     */
    virtual void emitEffect(const GrEffectStage&,
                            const GrEffectKey&,
                            const char* outColor,
                            const char* inColor,
                            int stageIndex) = 0;
};

////////////////////////////////////////////////////////////////////////////////

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
     * Helper for GrGLProgramEffectsBuilder::emitEfffect(). This method is meant to only be called
     * during the construction phase.
     */
    void emitEffect(GrGLFullShaderBuilder*,
                    const GrEffectStage&,
                    const GrEffectKey&,
                    const char* outColor,
                    const char* inColor,
                    int stageIndex);

    /**
     * Helper for emitEffect(). Emits any attributes an effect may have.
     */
    void emitAttributes(GrGLFullShaderBuilder*, const GrEffectStage&);

    /**
     * Helper for emitEffect(). Emits code to implement an effect's coord transforms in the VS.
     * Varyings are added as an outputs of the VS and inputs to the FS. The varyings may be either a
     * vec2f or vec3f depending upon whether perspective interpolation is required or not. The names
     * of the varyings in the VS and FS as well their types are appended to the
     * TransformedCoordsArray* object, which is in turn passed to the effect's emitCode() function.
     */
    void emitTransforms(GrGLFullShaderBuilder*,
                        const GrDrawEffect&,
                        TransformedCoordsArray*);

    /**
     * Helper for setData(). Sets all the transform matrices for an effect.
     */
    void setTransformData(const GrGLUniformManager&, const GrDrawEffect&, int effectIdx);

    struct Transform {
        Transform() { fCurrentValue = SkMatrix::InvalidMatrix(); }
        UniformHandle fHandle;
        SkMatrix      fCurrentValue;
    };

    SkTArray<SkSTArray<2, Transform, true> > fTransforms;
    bool                                     fHasExplicitLocalCoords;

    typedef GrGLProgramEffects INHERITED;
};

/**
 * This class is used to construct a GrGLVertexProgramEffects* object.
 */
class GrGLVertexProgramEffectsBuilder : public GrGLProgramEffectsBuilder {
public:
    GrGLVertexProgramEffectsBuilder(GrGLFullShaderBuilder*, int reserveCount);
    virtual ~GrGLVertexProgramEffectsBuilder() { }

    virtual void emitEffect(const GrEffectStage&,
                            const GrEffectKey&,
                            const char* outColor,
                            const char* inColor,
                            int stageIndex) SK_OVERRIDE;

    /**
     * Finalizes the building process and returns the effect array. After this call, the builder
     * becomes invalid.
     */
    GrGLProgramEffects* finish() { return fProgramEffects.detach(); }

private:
    GrGLFullShaderBuilder*                  fBuilder;
    SkAutoTDelete<GrGLVertexProgramEffects> fProgramEffects;

    typedef GrGLProgramEffectsBuilder INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

/**
 * This is a GrGLProgramEffects implementation that does coord transforms with
 * the the  NV_path_rendering PathTexGen functionality.
 */
class GrGLPathTexGenProgramEffects : public GrGLProgramEffects {
public:
    virtual void setData(GrGpuGL*,
                         const GrGLUniformManager&,
                         const GrEffectStage* effectStages[]) SK_OVERRIDE;

private:
    friend class GrGLPathTexGenProgramEffectsBuilder;

    GrGLPathTexGenProgramEffects(int reserveCount)
        : INHERITED(reserveCount)
        , fTransforms(reserveCount) {
    }

    /**
     * Helper for GrGLProgramEffectsBuilder::emitEfffect(). This method is meant to only be called
     * during the construction phase.
     */
    void emitEffect(GrGLFragmentOnlyShaderBuilder*,
                    const GrEffectStage&,
                    const GrEffectKey&,
                    const char* outColor,
                    const char* inColor,
                    int stageIndex);

    /**
     * Helper for emitEffect(). Allocates texture units from the builder for each transform in an
     * effect. The transforms all use adjacent texture units. They either use two or three of the
     * coordinates at a given texture unit, depending on if they need perspective interpolation.
     * The expressions to access the transformed coords (i.e. 'vec2(gl_TexCoord[0])') as well as the
     * types are appended to the TransformedCoordsArray* object, which is in turn passed to the
     * effect's emitCode() function.
     */
    void setupPathTexGen(GrGLFragmentOnlyShaderBuilder*,
                         const GrDrawEffect&,
                         TransformedCoordsArray*);

    /**
     * Helper for setData(). Sets the PathTexGen state for each transform in an effect.
     */
    void setPathTexGenState(GrGpuGL*, const GrDrawEffect&, int effectIdx);

    struct Transforms {
        Transforms(uint32_t transformKey, int texCoordIndex)
            : fTransformKey(transformKey), fTexCoordIndex(texCoordIndex) {}
        uint32_t    fTransformKey;
        int         fTexCoordIndex;
    };

    SkTArray<Transforms> fTransforms;

    typedef GrGLProgramEffects INHERITED;
};

/**
 * This class is used to construct a GrGLPathTexGenProgramEffects* object.
 */
class GrGLPathTexGenProgramEffectsBuilder : public GrGLProgramEffectsBuilder {
public:
    GrGLPathTexGenProgramEffectsBuilder(GrGLFragmentOnlyShaderBuilder*, int reserveCount);
    virtual ~GrGLPathTexGenProgramEffectsBuilder() { }

    virtual void emitEffect(const GrEffectStage&,
                            const GrEffectKey&,
                            const char* outColor,
                            const char* inColor,
                            int stageIndex) SK_OVERRIDE;

    /**
     * Finalizes the building process and returns the effect array. After this call, the builder
     * becomes invalid.
     */
    GrGLProgramEffects* finish() { return fProgramEffects.detach(); }

private:
    GrGLFragmentOnlyShaderBuilder*          fBuilder;
    SkAutoTDelete<GrGLPathTexGenProgramEffects> fProgramEffects;

    typedef GrGLProgramEffectsBuilder INHERITED;
};

#endif
