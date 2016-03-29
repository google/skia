/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgramBuilder.h"

#include "GrAutoLocaleSetter.h"
#include "GrCoordTransform.h"
#include "GrGLProgramBuilder.h"
#include "GrSwizzle.h"
#include "GrTexture.h"
#include "SkRTConf.h"
#include "SkTraceEvent.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLProgram.h"
#include "gl/GrGLProgramDesc.h"
#include "gl/GrGLSLPrettyPrint.h"
#include "gl/builders/GrGLShaderStringBuilder.h"
#include "glsl/GrGLSLCaps.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLTextureSampler.h"
#include "glsl/GrGLSLXferProcessor.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

GrGLProgram* GrGLProgramBuilder::CreateProgram(const GrPipeline& pipeline,
                                               const GrPrimitiveProcessor& primProc,
                                               const GrGLProgramDesc& desc,
                                               GrGLGpu* gpu) {
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrGLProgramBuilder builder(gpu, pipeline, primProc, desc);

    // TODO: Once all stages can handle taking a float or vec4 and correctly handling them we can
    // seed correctly here
    GrGLSLExpr4 inputColor;
    GrGLSLExpr4 inputCoverage;

    if (!builder.emitAndInstallProcs(&inputColor, &inputCoverage)) {
        builder.cleanupFragmentProcessors();
        return nullptr;
    }

    return builder.finalize();
}

/////////////////////////////////////////////////////////////////////////////

GrGLProgramBuilder::GrGLProgramBuilder(GrGLGpu* gpu,
                                       const GrPipeline& pipeline,
                                       const GrPrimitiveProcessor& primProc,
                                       const GrGLProgramDesc& desc)
    : INHERITED(pipeline, primProc, desc)
    , fGpu(gpu)
    , fVaryingHandler(this)
    , fUniformHandler(this) {
}

const GrCaps* GrGLProgramBuilder::caps() const {
    return fGpu->caps();
}

const GrGLSLCaps* GrGLProgramBuilder::glslCaps() const {
    return fGpu->ctxInfo().caps()->glslCaps();
}

bool GrGLProgramBuilder::compileAndAttachShaders(GrGLSLShaderBuilder& shader,
                                                 GrGLuint programId,
                                                 GrGLenum type,
                                                 SkTDArray<GrGLuint>* shaderIds) {
    GrGLGpu* gpu = this->gpu();
    GrGLuint shaderId = GrGLCompileAndAttachShader(gpu->glContext(),
                                                   programId,
                                                   type,
                                                   shader.fCompilerStrings.begin(),
                                                   shader.fCompilerStringLengths.begin(),
                                                   shader.fCompilerStrings.count(),
                                                   gpu->stats());

    if (!shaderId) {
        return false;
    }

    *shaderIds->append() = shaderId;

    return true;
}

GrGLProgram* GrGLProgramBuilder::finalize() {
    // verify we can get a program id
    GrGLuint programID;
    GL_CALL_RET(programID, CreateProgram());
    if (0 == programID) {
        this->cleanupFragmentProcessors();
        return nullptr;
    }

    this->finalizeShaders();

    // compile shaders and bind attributes / uniforms
    SkTDArray<GrGLuint> shadersToDelete;
    if (!this->compileAndAttachShaders(fVS, programID, GR_GL_VERTEX_SHADER, &shadersToDelete)) {
        this->cleanupProgram(programID, shadersToDelete);
        return nullptr;
    }

    // NVPR actually requires a vertex shader to compile
    bool useNvpr = primitiveProcessor().isPathRendering();
    if (!useNvpr) {
        const GrPrimitiveProcessor& primProc = this->primitiveProcessor();

        int vaCount = primProc.numAttribs();
        for (int i = 0; i < vaCount; i++) {
            GL_CALL(BindAttribLocation(programID, i, primProc.getAttrib(i).fName));
        }
    }

    if (!this->compileAndAttachShaders(fFS, programID, GR_GL_FRAGMENT_SHADER, &shadersToDelete)) {
        this->cleanupProgram(programID, shadersToDelete);
        return nullptr;
    }

    this->bindProgramResourceLocations(programID);

    GL_CALL(LinkProgram(programID));

    // Calling GetProgramiv is expensive in Chromium. Assume success in release builds.
    bool checkLinked = kChromium_GrGLDriver != fGpu->ctxInfo().driver();
#ifdef SK_DEBUG
    checkLinked = true;
#endif
    if (checkLinked) {
        checkLinkStatus(programID);
    }
    this->resolveProgramResourceLocations(programID);

    this->cleanupShaders(shadersToDelete);

    return this->createProgram(programID);
}

void GrGLProgramBuilder::bindProgramResourceLocations(GrGLuint programID) {
    fUniformHandler.bindUniformLocations(programID, fGpu->glCaps());

    const GrGLCaps& caps = this->gpu()->glCaps();
    if (fFS.hasCustomColorOutput() && caps.bindFragDataLocationSupport()) {
        GL_CALL(BindFragDataLocation(programID, 0,
                                     GrGLSLFragmentShaderBuilder::DeclaredColorOutputName()));
    }
    if (fFS.hasSecondaryOutput() && caps.glslCaps()->mustDeclareFragmentShaderOutput()) {
        GL_CALL(BindFragDataLocationIndexed(programID, 0, 1,
                                  GrGLSLFragmentShaderBuilder::DeclaredSecondaryColorOutputName()));
    }

    // handle NVPR separable varyings
    if (!fGpu->glCaps().shaderCaps()->pathRenderingSupport() ||
        !fGpu->glPathRendering()->shouldBindFragmentInputs()) {
        return;
    }
    int count = fVaryingHandler.fPathProcVaryingInfos.count();
    for (int i = 0; i < count; ++i) {
        GL_CALL(BindFragmentInputLocation(programID, i,
                                       fVaryingHandler.fPathProcVaryingInfos[i].fVariable.c_str()));
        fVaryingHandler.fPathProcVaryingInfos[i].fLocation = i;
    }
}

bool GrGLProgramBuilder::checkLinkStatus(GrGLuint programID) {
    GrGLint linked = GR_GL_INIT_ZERO;
    GL_CALL(GetProgramiv(programID, GR_GL_LINK_STATUS, &linked));
    if (!linked) {
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GL_CALL(GetProgramiv(programID, GR_GL_INFO_LOG_LENGTH, &infoLen));
        SkAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
        if (infoLen > 0) {
            // retrieve length even though we don't need it to workaround
            // bug in chrome cmd buffer param validation.
            GrGLsizei length = GR_GL_INIT_ZERO;
            GL_CALL(GetProgramInfoLog(programID,
                                      infoLen+1,
                                      &length,
                                      (char*)log.get()));
            SkDebugf("%s", (char*)log.get());
        }
        SkDEBUGFAIL("Error linking program");
        GL_CALL(DeleteProgram(programID));
        programID = 0;
    }
    return SkToBool(linked);
}

void GrGLProgramBuilder::resolveProgramResourceLocations(GrGLuint programID) {
    fUniformHandler.getUniformLocations(programID, fGpu->glCaps());

    // handle NVPR separable varyings
    if (!fGpu->glCaps().shaderCaps()->pathRenderingSupport() ||
        fGpu->glPathRendering()->shouldBindFragmentInputs()) {
        return;
    }
    int count = fVaryingHandler.fPathProcVaryingInfos.count();
    for (int i = 0; i < count; ++i) {
        GrGLint location;
        GL_CALL_RET(location, GetProgramResourceLocation(
                                       programID,
                                       GR_GL_FRAGMENT_INPUT,
                                       fVaryingHandler.fPathProcVaryingInfos[i].fVariable.c_str()));
        fVaryingHandler.fPathProcVaryingInfos[i].fLocation = location;
    }
}

void GrGLProgramBuilder::cleanupProgram(GrGLuint programID, const SkTDArray<GrGLuint>& shaderIDs) {
    GL_CALL(DeleteProgram(programID));
    this->cleanupShaders(shaderIDs);
    this->cleanupFragmentProcessors();
}
void GrGLProgramBuilder::cleanupShaders(const SkTDArray<GrGLuint>& shaderIDs) {
    for (int i = 0; i < shaderIDs.count(); ++i) {
      GL_CALL(DeleteShader(shaderIDs[i]));
    }
}

GrGLProgram* GrGLProgramBuilder::createProgram(GrGLuint programID) {
    return new GrGLProgram(fGpu,
                           this->desc(),
                           fUniformHandles,
                           programID,
                           fUniformHandler.fUniforms,
                           fVaryingHandler.fPathProcVaryingInfos,
                           fGeometryProcessor,
                           fXferProcessor,
                           fFragmentProcessors,
                           &fSamplerUniforms);
}
