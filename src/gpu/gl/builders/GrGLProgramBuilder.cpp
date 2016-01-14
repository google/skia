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

GrGLProgram* GrGLProgramBuilder::CreateProgram(const DrawArgs& args, GrGLGpu* gpu) {
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    SkAutoTDelete<GrGLProgramBuilder> builder(new GrGLProgramBuilder(gpu, args));

    GrGLProgramBuilder* pb = builder.get();

    // TODO: Once all stages can handle taking a float or vec4 and correctly handling them we can
    // seed correctly here
    GrGLSLExpr4 inputColor;
    GrGLSLExpr4 inputCoverage;

    if (!pb->emitAndInstallProcs(&inputColor,
                                 &inputCoverage,
                                 gpu->glCaps().maxFragmentTextureUnits())) {
        pb->cleanupFragmentProcessors();
        return nullptr;
    }

    return pb->finalize();
}

/////////////////////////////////////////////////////////////////////////////

GrGLProgramBuilder::GrGLProgramBuilder(GrGLGpu* gpu, const DrawArgs& args)
    : INHERITED(args)
    , fGpu(gpu)
    , fSamplerUniforms(4)
    , fVaryingHandler(this)
    , fUniformHandler(this) {
}

const GrCaps* GrGLProgramBuilder::caps() const {
    return fGpu->caps();
}

const GrGLSLCaps* GrGLProgramBuilder::glslCaps() const {
    return fGpu->ctxInfo().caps()->glslCaps();
}

static GrSLType get_sampler_type(const GrTextureAccess& access) {
    GrGLTexture* glTexture = static_cast<GrGLTexture*>(access.getTexture());
    if (glTexture->target() == GR_GL_TEXTURE_EXTERNAL) {
        return kSamplerExternal_GrSLType;
    } else if (glTexture->target() == GR_GL_TEXTURE_RECTANGLE) {
        return kSampler2DRect_GrSLType;
    } else {
        SkASSERT(glTexture->target() == GR_GL_TEXTURE_2D);
        return kSampler2D_GrSLType;
    }
}

void GrGLProgramBuilder::emitSamplers(const GrProcessor& processor,
                                      GrGLSLTextureSampler::TextureSamplerArray* outSamplers) {
    int numTextures = processor.numTextures();
    UniformHandle* localSamplerUniforms = fSamplerUniforms.push_back_n(numTextures);
    SkString name;
    for (int t = 0; t < numTextures; ++t) {
        name.printf("Sampler%d", t);
        GrSLType samplerType = get_sampler_type(processor.textureAccess(t));
        localSamplerUniforms[t] =
            fUniformHandler.addUniform(GrGLSLUniformHandler::kFragment_Visibility,
                                       samplerType, kDefault_GrSLPrecision,
                                       name.c_str());
        SkNEW_APPEND_TO_TARRAY(outSamplers, GrGLSLTextureSampler,
                               (localSamplerUniforms[t], processor.textureAccess(t)));
        if (kSamplerExternal_GrSLType == samplerType) {
            const char* externalFeatureString = this->glslCaps()->externalTextureExtensionString();
            // We shouldn't ever create a GrGLTexture that requires external sampler type 
            SkASSERT(externalFeatureString);
            fFS.addFeature(1 << GrGLSLFragmentShaderBuilder::kExternalTexture_GLSLPrivateFeature,
                           externalFeatureString);
        }
    }
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

    // compile shaders and bind attributes / uniforms
    SkTDArray<GrGLuint> shadersToDelete;
    fVS.finalize(GrGLSLUniformHandler::kVertex_Visibility);
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

    fFS.finalize(GrGLSLUniformHandler::kFragment_Visibility);
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
        !fGpu->glPathRendering()->shouldBindFragmentInputs()) {
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

