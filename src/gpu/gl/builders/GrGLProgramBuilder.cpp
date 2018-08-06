/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgramBuilder.h"

#include "GrAutoLocaleSetter.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrCoordTransform.h"
#include "GrGLProgramBuilder.h"
#include "GrProgramDesc.h"
#include "GrShaderCaps.h"
#include "GrSwizzle.h"
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

GrGLProgram* GrGLProgramBuilder::CreateProgram(const GrPrimitiveProcessor& primProc,
                                               const GrPipeline& pipeline,
                                               GrProgramDesc* desc,
                                               GrGLGpu* gpu) {
#ifdef SK_DEBUG
    GrResourceProvider* resourceProvider = gpu->getContext()->contextPriv().resourceProvider();

    SkASSERT(!pipeline.isBad() && primProc.instantiate(resourceProvider));
#endif

    ATRACE_ANDROID_FRAMEWORK("Shader Compile");
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrGLProgramBuilder builder(gpu, pipeline, primProc, desc);

    auto persistentCache = gpu->getContext()->contextPriv().getPersistentCache();
    if (persistentCache && gpu->glCaps().programBinarySupport()) {
        sk_sp<SkData> key = SkData::MakeWithoutCopy(desc->asKey(), desc->keyLength());
        builder.fCached = persistentCache->load(*key);
        // the eventual end goal is to completely skip emitAndInstallProcs on a cache hit, but it's
        // doing necessary setup in addition to generating the SkSL code. Currently we are only able
        // to skip the SkSL->GLSL step on a cache hit.
    }
    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }
    return builder.finalize();
}

/////////////////////////////////////////////////////////////////////////////

GrGLProgramBuilder::GrGLProgramBuilder(GrGLGpu* gpu,
                                       const GrPipeline& pipeline,
                                       const GrPrimitiveProcessor& primProc,
                                       GrProgramDesc* desc)
        : INHERITED(primProc, pipeline, desc)
        , fGpu(gpu)
        , fVaryingHandler(this)
        , fUniformHandler(this)
        , fVertexAttributeCnt(0)
        , fInstanceAttributeCnt(0)
        , fVertexStride(0)
        , fInstanceStride(0) {}

const GrCaps* GrGLProgramBuilder::caps() const {
    return fGpu->caps();
}

bool GrGLProgramBuilder::compileAndAttachShaders(const char* glsl,
                                                 int length,
                                                 GrGLuint programId,
                                                 GrGLenum type,
                                                 SkTDArray<GrGLuint>* shaderIds,
                                                 const SkSL::Program::Settings& settings,
                                                 const SkSL::Program::Inputs& inputs) {
    GrGLGpu* gpu = this->gpu();
    GrGLuint shaderId = GrGLCompileAndAttachShader(gpu->glContext(),
                                                   programId,
                                                   type,
                                                   glsl,
                                                   length,
                                                   gpu->stats(),
                                                   settings);
    if (!shaderId) {
        return false;
    }

    *shaderIds->append() = shaderId;
    if (inputs.fFlipY) {
        GrProgramDesc* d = this->desc();
        d->setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(
                                                     this->pipeline().proxy()->origin()));
        d->finalize();
    }

    return true;
}

bool GrGLProgramBuilder::compileAndAttachShaders(GrGLSLShaderBuilder& shader,
                                                 GrGLuint programId,
                                                 GrGLenum type,
                                                 SkTDArray<GrGLuint>* shaderIds,
                                                 const SkSL::Program::Settings& settings,
                                                 SkSL::Program::Inputs* outInputs) {
    SkSL::String glsl;
    std::unique_ptr<SkSL::Program> program = GrSkSLtoGLSL(gpu()->glContext(), type,
                                                 shader.fCompilerStrings.begin(),
                                                 shader.fCompilerStringLengths.begin(),
                                                 shader.fCompilerStrings.count(),
                                                 settings,
                                                 &glsl);
    *outInputs = program->fInputs;
    return this->compileAndAttachShaders(glsl.c_str(),
                                         glsl.size(),
                                         programId,
                                         type,
                                         shaderIds,
                                         settings,
                                         *outInputs);
}

void GrGLProgramBuilder::computeCountsAndStrides(GrGLuint programID,
                                                 const GrPrimitiveProcessor& primProc,
                                                 bool bindAttribLocations) {
    fVertexAttributeCnt = primProc.numVertexAttributes();
    fInstanceAttributeCnt = primProc.numInstanceAttributes();
    fAttributes.reset(
            new GrGLProgram::Attribute[fVertexAttributeCnt + fInstanceAttributeCnt]);
    auto addAttr = [&](int i, const auto& a, size_t* stride) {
        fAttributes[i].fType = a.type();
        fAttributes[i].fOffset = *stride;
        *stride += a.sizeAlign4();
        fAttributes[i].fLocation = i;
        if (bindAttribLocations) {
            GL_CALL(BindAttribLocation(programID, i, a.name()));
        }
    };
    fVertexStride = 0;
    int i = 0;
    for (; i < fVertexAttributeCnt; i++) {
        addAttr(i, primProc.vertexAttribute(i), &fVertexStride);
        SkASSERT(fAttributes[i].fOffset == primProc.debugOnly_vertexAttributeOffset(i));
    }
    SkASSERT(fVertexStride == primProc.debugOnly_vertexStride());
    fInstanceStride = 0;
    for (int j = 0; j < fInstanceAttributeCnt; j++, ++i) {
        addAttr(i, primProc.instanceAttribute(j), &fInstanceStride);
        SkASSERT(fAttributes[i].fOffset == primProc.debugOnly_instanceAttributeOffset(j));
    }
    SkASSERT(fInstanceStride == primProc.debugOnly_instanceStride());
}

GrGLProgram* GrGLProgramBuilder::finalize() {
    TRACE_EVENT0("skia", TRACE_FUNC);

    // verify we can get a program id
    GrGLuint programID;
    GL_CALL_RET(programID, CreateProgram());
    if (0 == programID) {
        return nullptr;
    }

    if (this->gpu()->glCaps().programBinarySupport() &&
        this->gpu()->getContext()->contextPriv().getPersistentCache()) {
        GL_CALL(ProgramParameteri(programID, GR_GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GR_GL_TRUE));
    }

    this->finalizeShaders();

    // compile shaders and bind attributes / uniforms
    const GrPrimitiveProcessor& primProc = this->primitiveProcessor();
    SkSL::Program::Settings settings;
    settings.fCaps = this->gpu()->glCaps().shaderCaps();
    settings.fFlipY = this->pipeline().proxy()->origin() != kTopLeft_GrSurfaceOrigin;
    settings.fSharpenTextures = this->gpu()->getContext()->contextPriv().sharpenMipmappedTextures();
    settings.fFragColorIsInOut = this->fragColorIsInOut();

    SkSL::Program::Inputs inputs;
    SkTDArray<GrGLuint> shadersToDelete;
    bool cached = fGpu->glCaps().programBinarySupport() && nullptr != fCached.get();
    if (cached) {
        this->bindProgramResourceLocations(programID);
        // cache hit, just hand the binary to GL
        const uint8_t* bytes = fCached->bytes();
        size_t offset = 0;
        memcpy(&inputs, bytes + offset, sizeof(inputs));
        offset += sizeof(inputs);
        int binaryFormat;
        memcpy(&binaryFormat, bytes + offset, sizeof(binaryFormat));
        offset += sizeof(binaryFormat);
        GrGLClearErr(this->gpu()->glInterface());
        GR_GL_CALL_NOERRCHECK(this->gpu()->glInterface(),
                              ProgramBinary(programID, binaryFormat, (void*) (bytes + offset),
                                            fCached->size() - offset));
        if (GR_GL_GET_ERROR(this->gpu()->glInterface()) == GR_GL_NO_ERROR) {
            cached = this->checkLinkStatus(programID);
            if (cached) {
                if (inputs.fRTHeight) {
                    this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
                }
                this->computeCountsAndStrides(programID, primProc, false);
            }
        } else {
            cached = false;
        }
    }
    if (!cached) {
        // cache miss, compile shaders
        if (fFS.fForceHighPrecision) {
            settings.fForceHighPrecision = true;
        }
        SkSL::String glsl;
        std::unique_ptr<SkSL::Program> fs = GrSkSLtoGLSL(gpu()->glContext(),
                                                         GR_GL_FRAGMENT_SHADER,
                                                         fFS.fCompilerStrings.begin(),
                                                         fFS.fCompilerStringLengths.begin(),
                                                         fFS.fCompilerStrings.count(),
                                                         settings,
                                                         &glsl);
        if (!fs) {
            this->cleanupProgram(programID, shadersToDelete);
            return nullptr;
        }
        inputs = fs->fInputs;
        if (inputs.fRTHeight) {
            this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
        }
        if (!this->compileAndAttachShaders(glsl.c_str(), glsl.size(), programID,
                                           GR_GL_FRAGMENT_SHADER, &shadersToDelete, settings,
                                           inputs)) {
            this->cleanupProgram(programID, shadersToDelete);
            return nullptr;
        }

        std::unique_ptr<SkSL::Program> vs = GrSkSLtoGLSL(gpu()->glContext(),
                                                         GR_GL_VERTEX_SHADER,
                                                         fVS.fCompilerStrings.begin(),
                                                         fVS.fCompilerStringLengths.begin(),
                                                         fVS.fCompilerStrings.count(),
                                                         settings,
                                                         &glsl);
        if (!vs || !this->compileAndAttachShaders(glsl.c_str(), glsl.size(), programID,
                                                  GR_GL_VERTEX_SHADER, &shadersToDelete, settings,
                                                  inputs)) {
            this->cleanupProgram(programID, shadersToDelete);
            return nullptr;
        }

        // NVPR actually requires a vertex shader to compile
        bool useNvpr = primProc.isPathRendering();
        if (!useNvpr) {
            this->computeCountsAndStrides(programID, primProc, true);
        }

        if (primProc.willUseGeoShader()) {
            std::unique_ptr<SkSL::Program> gs;
            gs = GrSkSLtoGLSL(gpu()->glContext(),
                              GR_GL_GEOMETRY_SHADER,
                              fGS.fCompilerStrings.begin(),
                              fGS.fCompilerStringLengths.begin(),
                              fGS.fCompilerStrings.count(),
                              settings,
                              &glsl);
            if (!gs || !this->compileAndAttachShaders(glsl.c_str(), glsl.size(), programID,
                                                      GR_GL_GEOMETRY_SHADER, &shadersToDelete,
                                                      settings, inputs)) {
                this->cleanupProgram(programID, shadersToDelete);
                return nullptr;
            }
        }
        this->bindProgramResourceLocations(programID);

        GL_CALL(LinkProgram(programID));
    }
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
            return nullptr;
        }
    }
    this->resolveProgramResourceLocations(programID);

    this->cleanupShaders(shadersToDelete);
    if (!cached && this->gpu()->getContext()->contextPriv().getPersistentCache() &&
        fGpu->glCaps().programBinarySupport()) {
        GrGLsizei length = 0;
        GL_CALL(GetProgramiv(programID, GL_PROGRAM_BINARY_LENGTH, &length));
        if (length > 0) {
            // store shader in cache
            sk_sp<SkData> key = SkData::MakeWithoutCopy(desc()->asKey(), desc()->keyLength());
            GrGLenum binaryFormat;
            std::unique_ptr<char[]> binary(new char[length]);
            GL_CALL(GetProgramBinary(programID, length, &length, &binaryFormat, binary.get()));
            size_t dataLength = sizeof(inputs) + sizeof(binaryFormat) + length;
            std::unique_ptr<uint8_t[]> data(new uint8_t[dataLength]);
            size_t offset = 0;
            memcpy(data.get() + offset, &inputs, sizeof(inputs));
            offset += sizeof(inputs);
            memcpy(data.get() + offset, &binaryFormat, sizeof(binaryFormat));
            offset += sizeof(binaryFormat);
            memcpy(data.get() + offset, binary.get(), length);
            this->gpu()->getContext()->contextPriv().getPersistentCache()->store(
                                            *key, *SkData::MakeWithoutCopy(data.get(), dataLength));
        }
    }
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
}
void GrGLProgramBuilder::cleanupShaders(const SkTDArray<GrGLuint>& shaderIDs) {
    for (int i = 0; i < shaderIDs.count(); ++i) {
        GL_CALL(DeleteShader(shaderIDs[i]));
    }
}

GrGLProgram* GrGLProgramBuilder::createProgram(GrGLuint programID) {
    return new GrGLProgram(fGpu,
                           fUniformHandles,
                           programID,
                           fUniformHandler.fUniforms,
                           fUniformHandler.fSamplers,
                           fVaryingHandler.fPathProcVaryingInfos,
                           std::move(fGeometryProcessor),
                           std::move(fXferProcessor),
                           std::move(fFragmentProcessors),
                           fFragmentProcessorCnt,
                           std::move(fAttributes),
                           fVertexAttributeCnt,
                           fInstanceAttributeCnt,
                           fVertexStride,
                           fInstanceStride);
}
