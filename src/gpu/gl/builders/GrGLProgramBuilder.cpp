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
#include "GrProgramDesc.h"
#include "GrShaderCaps.h"
#include "GrSwizzle.h"
#include "GrTexture.h"
#include "SkAutoMalloc.h"
#include "SkATrace.h"
#include "SkTraceEvent.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLProgram.h"
#include "gl/builders/GrGLShaderStringBuilder.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLXferProcessor.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

GrGLProgram* GrGLProgramBuilder::CreateProgram(const GrPipeline& pipeline,
                                               const GrPrimitiveProcessor& primProc,
                                               GrProgramDesc* desc,
                                               GrGLGpu* gpu) {
    SkASSERT(!pipeline.isBad() && !primProc.isBad());

    ATRACE_ANDROID_FRAMEWORK("Shader Compile");
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrGLProgramBuilder builder(gpu, pipeline, primProc, desc);

    if (!builder.emitAndInstallProcs()) {
        builder.cleanupFragmentProcessors();
        return nullptr;
    }

    return builder.finalize();
}

/////////////////////////////////////////////////////////////////////////////

GrGLProgramBuilder::GrGLProgramBuilder(GrGLGpu* gpu,
                                       const GrPipeline& pipeline,
                                       const GrPrimitiveProcessor& primProc,
                                       GrProgramDesc* desc)
    : INHERITED(pipeline, primProc, desc)
    , fGpu(gpu)
    , fVaryingHandler(this)
    , fUniformHandler(this) {
}

const GrCaps* GrGLProgramBuilder::caps() const {
    return fGpu->caps();
}

bool GrGLProgramBuilder::compileAndAttachShaders(GrGLSLShaderBuilder& shader,
                                                 GrGLuint programId,
                                                 GrGLenum type,
                                                 SkTDArray<GrGLuint>* shaderIds,
                                                 const SkSL::Program::Settings& settings,
                                                 SkSL::Program::Inputs* outInputs) {
    GrGLGpu* gpu = this->gpu();
    GrGLuint shaderId = GrGLCompileAndAttachShader(gpu->glContext(),
                                                   programId,
                                                   type,
                                                   shader.fCompilerStrings.begin(),
                                                   shader.fCompilerStringLengths.begin(),
                                                   shader.fCompilerStrings.count(),
                                                   gpu->stats(),
                                                   settings,
                                                   outInputs);

    if (!shaderId) {
        return false;
    }

    *shaderIds->append() = shaderId;
    if (outInputs->fFlipY) {
        GrProgramDesc* d = this->desc();
        d->setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(
                                                     this->pipeline().getRenderTarget()->origin()));
        d->finalize();
    }

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
    SkSL::Program::Settings settings;
    settings.fCaps = this->gpu()->glCaps().shaderCaps();
    settings.fFlipY = this->pipeline().getRenderTarget()->origin() != kTopLeft_GrSurfaceOrigin;
    SkSL::Program::Inputs inputs;
    SkTDArray<GrGLuint> shadersToDelete;
    if (!this->compileAndAttachShaders(fVS, programID, GR_GL_VERTEX_SHADER, &shadersToDelete,
                                       settings, &inputs)) {
        this->cleanupProgram(programID, shadersToDelete);
        return nullptr;
    }

    // NVPR actually requires a vertex shader to compile
    const GrPrimitiveProcessor& primProc = this->primitiveProcessor();
    bool useNvpr = primProc.isPathRendering();
    if (!useNvpr) {
        int vaCount = primProc.numAttribs();
        for (int i = 0; i < vaCount; i++) {
            GL_CALL(BindAttribLocation(programID, i, primProc.getAttrib(i).fName));
        }
    }

    if (primProc.willUseGeoShader() &&
        !this->compileAndAttachShaders(fGS, programID, GR_GL_GEOMETRY_SHADER, &shadersToDelete,
                                       settings, &inputs)) {
        this->cleanupProgram(programID, shadersToDelete);
        return nullptr;
    }

    if (!this->compileAndAttachShaders(fFS, programID, GR_GL_FRAGMENT_SHADER, &shadersToDelete,
                                       settings, &inputs)) {
        this->cleanupProgram(programID, shadersToDelete);
        return nullptr;
    }

    if (inputs.fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }

    this->bindProgramResourceLocations(programID);

    GL_CALL(LinkProgram(programID));

    // Calling GetProgramiv is expensive in Chromium. Assume success in release builds.
    bool checkLinked = kChromium_GrGLDriver != fGpu->ctxInfo().driver();
#ifdef SK_DEBUG
    checkLinked = true;
#endif
    if (checkLinked) {
        if (!this->checkLinkStatus(programID)) {
            SkDebugf("VS:\n");
            GrGLPrintShader(fGpu->glContext(), GR_GL_VERTEX_SHADER, fVS.fCompilerStrings.begin(),
                            fVS.fCompilerStringLengths.begin(), fVS.fCompilerStrings.count(),
                            settings);
            if (primProc.willUseGeoShader()) {
                SkDebugf("\nGS:\n");
                GrGLPrintShader(fGpu->glContext(), GR_GL_GEOMETRY_SHADER,
                                fGS.fCompilerStrings.begin(), fGS.fCompilerStringLengths.begin(),
                                fGS.fCompilerStrings.count(), settings);
            }
            SkDebugf("\nFS:\n");
            GrGLPrintShader(fGpu->glContext(), GR_GL_FRAGMENT_SHADER, fFS.fCompilerStrings.begin(),
                            fFS.fCompilerStringLengths.begin(), fFS.fCompilerStrings.count(),
                            settings);
            SkDEBUGFAIL("");
            return nullptr;
        }
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
    if (fFS.hasSecondaryOutput() && caps.shaderCaps()->mustDeclareFragmentShaderOutput()) {
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
        SkDebugf("Program linking failed.\n");
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
                           *this->desc(),
                           fUniformHandles,
                           programID,
                           fUniformHandler.fUniforms,
                           fUniformHandler.fSamplers,
                           fUniformHandler.fTexelBuffers,
                           fUniformHandler.fImageStorages,
                           fVaryingHandler.fPathProcVaryingInfos,
                           fGeometryProcessor,
                           fXferProcessor,
                           fFragmentProcessors);
}
