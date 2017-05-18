/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLXferProcessor_DEFINED
#define GrGLSLXferProcessor_DEFINED

#include "SkPoint.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

class GrXferProcessor;
class GrGLSLXPBuilder;
class GrGLSLXPFragmentBuilder;
class GrShaderCaps;
class GrTexture;

class GrGLSLXferProcessor {
public:
    GrGLSLXferProcessor() {}
    virtual ~GrGLSLXferProcessor() {}

    using SamplerHandle = GrGLSLUniformHandler::SamplerHandle;
    using ImageStorageHandle = GrGLSLUniformHandler::ImageStorageHandle;

    struct EmitArgs {
        EmitArgs(GrGLSLXPFragmentBuilder* fragBuilder,
                 GrGLSLUniformHandler* uniformHandler,
                 const GrShaderCaps* caps,
                 const GrXferProcessor& xp,
                 const char* inputColor,
                 const char* inputCoverage,
                 const char* outputPrimary,
                 const char* outputSecondary,
                 const SamplerHandle dstTextureSamplerHandle,
                 GrSurfaceOrigin dstTextureOrigin)
                : fXPFragBuilder(fragBuilder)
                , fUniformHandler(uniformHandler)
                , fShaderCaps(caps)
                , fXP(xp)
                , fInputColor(inputColor)
                , fInputCoverage(inputCoverage)
                , fOutputPrimary(outputPrimary)
                , fOutputSecondary(outputSecondary)
                , fDstTextureSamplerHandle(dstTextureSamplerHandle)
                , fDstTextureOrigin(dstTextureOrigin) {}
        GrGLSLXPFragmentBuilder* fXPFragBuilder;
        GrGLSLUniformHandler* fUniformHandler;
        const GrShaderCaps* fShaderCaps;
        const GrXferProcessor& fXP;
        const char* fInputColor;
        const char* fInputCoverage;
        const char* fOutputPrimary;
        const char* fOutputSecondary;
        const SamplerHandle fDstTextureSamplerHandle;
        GrSurfaceOrigin fDstTextureOrigin;
    };
    /**
     * This is similar to emitCode() in the base class, except it takes a full shader builder.
     * This allows the effect subclass to emit vertex code.
     */
    void emitCode(const EmitArgs&);

    /** A GrGLSLXferProcessor instance can be reused with any GrGLSLXferProcessor that produces
        the same stage key; this function reads data from a GrGLSLXferProcessor and uploads any
        uniform variables required  by the shaders created in emitCode(). The GrXferProcessor
        parameter is guaranteed to be of the same type that created this GrGLSLXferProcessor and
        to have an identical processor key as the one that created this GrGLSLXferProcessor. This
        function calls onSetData on the subclass of GrGLSLXferProcessor
     */
    void setData(const GrGLSLProgramDataManager& pdm, const GrXferProcessor& xp,
                 const GrTexture* dstTexture, const SkIPoint& dstTextureOffset);

protected:
    static void DefaultCoverageModulation(GrGLSLXPFragmentBuilder* fragBuilder,
                                          const char* srcCoverage,
                                          const char* dstColor,
                                          const char* outColor,
                                          const char* outColorSecondary,
                                          const GrXferProcessor& proc);

private:
    /**
     * Called by emitCode() when the XP will not be performing a dst read. This method is
     * responsible for both blending and coverage. A subclass only needs to implement this method if
     * it can construct a GrXferProcessor that will not read the dst color.
     */
    virtual void emitOutputsForBlendState(const EmitArgs&) {
        SkFAIL("emitOutputsForBlendState not implemented.");
    }

    /**
     * Called by emitCode() when the XP will perform a dst read. This method only needs to supply
     * the blending logic. The base class applies coverage. A subclass only needs to implement this
     * method if it can construct a GrXferProcessor that reads the dst color.
     */
    virtual void emitBlendCodeForDstRead(GrGLSLXPFragmentBuilder*,
                                         GrGLSLUniformHandler*,
                                         const char* srcColor,
                                         const char* srcCoverage,
                                         const char* dstColor,
                                         const char* outColor,
                                         const char* outColorSecondary,
                                         const GrXferProcessor&) {
        SkFAIL("emitBlendCodeForDstRead not implemented.");
    }

    virtual void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) = 0;

    GrGLSLProgramDataManager::UniformHandle fDstTopLeftUni;
    GrGLSLProgramDataManager::UniformHandle fDstScaleUni;
};
#endif
