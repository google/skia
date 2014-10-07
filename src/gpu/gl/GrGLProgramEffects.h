/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLProgramEffects_DEFINED
#define GrGLProgramEffects_DEFINED

#include "GrBackendProcessorFactory.h"
#include "GrGLProgramDataManager.h"
#include "GrGpu.h"
#include "GrTexture.h"
#include "GrTextureAccess.h"

class GrProcessor;
class GrProcessorStage;
class GrGLVertexProgramEffectsBuilder;
class GrGLProgramBuilder;
class GrGLFullProgramBuilder;
class GrGLFragmentOnlyProgramBuilder;

/**
 * This class encapsulates an array of GrGLProcessors and their supporting data (coord transforms
 * and textures). It is built with GrGLProgramEffectsBuilder, then used to manage the necessary GL
 * state and shader uniforms.
 */
class GrGLProgramEffects : public SkRefCnt {
public:
    typedef GrGLProgramDataManager::UniformHandle UniformHandle;
    typedef GrGLProgramDataManager::VaryingHandle VaryingHandle;
    virtual ~GrGLProgramEffects();

    /**
     * Assigns a texture unit to each sampler. It starts on *texUnitIdx and writes the next
     * available unit to *texUnitIdx when it returns.
     */
    void initSamplers(const GrGLProgramDataManager&, int* texUnitIdx);

    /**
     * Calls setData() on each effect, and sets their transformation matrices and texture bindings.
     */
    virtual void setData(GrGpuGL*,
                         GrGpu::DrawType,
                         const GrGLProgramDataManager&,
                         const GrGeometryStage* effectStages) {
        SkFAIL("For geometry processor only");
    }

    virtual void setData(GrGpuGL*,
                         GrGpu::DrawType,
                         const GrGLProgramDataManager&,
                         const GrFragmentStage* effectStages[]) = 0;

protected:
    GrGLProgramEffects(int reserveCount)
        : fGLProcessors(reserveCount)
        , fSamplers(reserveCount) {
    }

    /**
     * Helper for setData(). Binds all the textures for an effect.
     */
    void bindTextures(GrGpuGL*, const GrProcessor&, int effectIdx);

    struct Sampler {
        SkDEBUGCODE(Sampler() : fTextureUnit(-1) {})
        UniformHandle fUniform;
        int           fTextureUnit;
    };

    /*
     * Helpers for shader builders to build up program effects objects alongside shader code
     */
    void addEffect(GrGLProcessor* effect) { fGLProcessors.push_back(effect); }
    SkTArray<Sampler, true>& addSamplers() { return fSamplers.push_back(); }

    SkTArray<GrGLProcessor*>               fGLProcessors;
    SkTArray<SkSTArray<4, Sampler, true> > fSamplers;

private:
    friend class GrGLProgramBuilder;
    friend class GrGLFullProgramBuilder;
    friend class GrGLFragmentOnlyShaderBuilder;

    typedef SkRefCnt INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

/**
 * This is a GrGLProgramEffects implementation that does coord transforms with the vertex shader.
 */
class GrGLVertexProgramEffects : public GrGLProgramEffects {
public:
    virtual void setData(GrGpuGL*,
                         GrGpu::DrawType,
                         const GrGLProgramDataManager&,
                         const GrGeometryStage* effectStages) SK_OVERRIDE;

    virtual void setData(GrGpuGL*,
                         GrGpu::DrawType,
                         const GrGLProgramDataManager&,
                         const GrFragmentStage* effectStages[]) SK_OVERRIDE;

private:
    GrGLVertexProgramEffects(int reserveCount, bool explicitLocalCoords)
        : INHERITED(reserveCount)
        , fTransforms(reserveCount)
        , fHasExplicitLocalCoords(explicitLocalCoords) {
    }

    struct Transform {
        Transform() { fCurrentValue = SkMatrix::InvalidMatrix(); }
        UniformHandle fHandle;
        SkMatrix      fCurrentValue;
    };

    struct PathTransform {
        PathTransform() { fCurrentValue = SkMatrix::InvalidMatrix(); }
        VaryingHandle fHandle;
        SkMatrix fCurrentValue;
        GrSLType fType;
    };

    /*
     * These functions are used by the builders to build up program effects along side the shader
     * code itself
     */
    SkSTArray<2, Transform, true>& addTransforms() { return fTransforms.push_back(); }
    SkTArray<PathTransform, true>& addPathTransforms() { return fPathTransforms.push_back(); }

    /**
     * Helper for setData(). Sets all the transform matrices for an effect.
     */
    void setDataInternal(GrGpuGL* gpu,
                         GrGpu::DrawType drawType,
                         const GrGLProgramDataManager& programDataManager,
                         const GrProcessorStage& effectStage,
                         int index);
    void setTransformData(GrGpuGL* gpu, const GrGLProgramDataManager&, const GrProcessorStage&,
                          int effectIdx);
    void setPathTransformData(GrGpuGL* gpu, const GrGLProgramDataManager&,
                              const GrProcessorStage&, int effectIdx);


    SkTArray<SkSTArray<2, Transform, true> > fTransforms;
    SkTArray<SkTArray<PathTransform, true> > fPathTransforms;
    bool                                     fHasExplicitLocalCoords;

    friend class GrGLFullProgramBuilder;

    typedef GrGLProgramEffects INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

/**
 * This is a GrGLProgramEffects implementation that does coord transforms with
 * the the  NV_path_rendering PathTexGen functionality.
 */
class GrGLPathTexGenProgramEffects : public GrGLProgramEffects {
public:
    virtual void setData(GrGpuGL*,
                         GrGpu::DrawType,
                         const GrGLProgramDataManager&,
                         const GrFragmentStage* effectStages[]) SK_OVERRIDE;

private:
    GrGLPathTexGenProgramEffects(int reserveCount)
        : INHERITED(reserveCount)
        , fTransforms(reserveCount) {
    }

    /**
     * Helper for setData(). Sets the PathTexGen state for each transform in an effect.
     */
    void setPathTexGenState(GrGpuGL*, const GrProcessorStage&, int effectIdx);

    struct Transforms {
        Transforms(int texCoordIndex)
            : fTexCoordIndex(texCoordIndex) {}
        int fTexCoordIndex;
    };

    /*
     * Helper for fragment only shader builder to build up the program effects alongside the shader
     */
    void addTransforms(int coordIndex) {
        fTransforms.push_back(Transforms(coordIndex));
    }

    SkTArray<Transforms> fTransforms;

    friend class GrGLFragmentOnlyProgramBuilder;

    typedef GrGLProgramEffects INHERITED;
};

#endif
