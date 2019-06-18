/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/builders/GrGLProgramBuilder.h"

#include "include/gpu/GrContext.h"
#include "src/core/SkATrace.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkReader32.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkWriter32.h"
#include "src/gpu/GrAutoLocaleSetter.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrPersistentCacheUtils.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/GrSwizzle.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLProgram.h"
#include "src/gpu/gl/builders/GrGLProgramBuilder.h"
#include "src/gpu/gl/builders/GrGLShaderStringBuilder.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

GrGLProgram* GrGLProgramBuilder::CreateProgram(GrRenderTarget* renderTarget, GrSurfaceOrigin origin,
                                               const GrPrimitiveProcessor& primProc,
                                               const GrTextureProxy* const primProcProxies[],
                                               const GrPipeline& pipeline,
                                               GrProgramDesc* desc,
                                               GrGLGpu* gpu) {
    SkASSERT(!pipeline.isBad());

    ATRACE_ANDROID_FRAMEWORK("Shader Compile");
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrGLProgramBuilder builder(gpu, renderTarget, origin,
                               pipeline, primProc, primProcProxies, desc);

    auto persistentCache = gpu->getContext()->priv().getPersistentCache();
    if (persistentCache) {
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
                                       GrRenderTarget* renderTarget,
                                       GrSurfaceOrigin origin,
                                       const GrPipeline& pipeline,
                                       const GrPrimitiveProcessor& primProc,
                                       const GrTextureProxy* const primProcProxies[],
                                       GrProgramDesc* desc)
        : INHERITED(renderTarget, origin, primProc, primProcProxies, pipeline, desc)
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

bool GrGLProgramBuilder::compileAndAttachShaders(const SkSL::String& glsl,
                                                 GrGLuint programId,
                                                 GrGLenum type,
                                                 SkTDArray<GrGLuint>* shaderIds,
                                                 const SkSL::Program::Inputs& inputs,
                                                 GrContextOptions::ShaderErrorHandler* errHandler) {
    GrGLGpu* gpu = this->gpu();
    GrGLuint shaderId = GrGLCompileAndAttachShader(gpu->glContext(),
                                                   programId,
                                                   type,
                                                   glsl,
                                                   gpu->stats(),
                                                   errHandler);
    if (!shaderId) {
        return false;
    }

    *shaderIds->append() = shaderId;
    if (inputs.fFlipY) {
        GrProgramDesc* d = this->desc();
        d->setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(this->origin()));
    }

    return true;
}

void GrGLProgramBuilder::computeCountsAndStrides(GrGLuint programID,
                                                 const GrPrimitiveProcessor& primProc,
                                                 bool bindAttribLocations) {
    fVertexAttributeCnt = primProc.numVertexAttributes();
    fInstanceAttributeCnt = primProc.numInstanceAttributes();
    fAttributes.reset(
            new GrGLProgram::Attribute[fVertexAttributeCnt + fInstanceAttributeCnt]);
    auto addAttr = [&](int i, const auto& a, size_t* stride) {
        fAttributes[i].fCPUType = a.cpuType();
        fAttributes[i].fGPUType = a.gpuType();
        fAttributes[i].fOffset = *stride;
        *stride += a.sizeAlign4();
        fAttributes[i].fLocation = i;
        if (bindAttribLocations) {
            GL_CALL(BindAttribLocation(programID, i, a.name()));
        }
    };
    fVertexStride = 0;
    int i = 0;
    for (const auto& attr : primProc.vertexAttributes()) {
        addAttr(i++, attr, &fVertexStride);
    }
    SkASSERT(fVertexStride == primProc.vertexStride());
    fInstanceStride = 0;
    for (const auto& attr : primProc.instanceAttributes()) {
        addAttr(i++, attr, &fInstanceStride);
    }
    SkASSERT(fInstanceStride == primProc.instanceStride());
}

void GrGLProgramBuilder::addInputVars(const SkSL::Program::Inputs& inputs) {
    if (inputs.fRTWidth) {
        this->addRTWidthUniform(SKSL_RTWIDTH_NAME);
    }
    if (inputs.fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }
}

static constexpr SkFourByteTag kSKSL_Tag = SkSetFourByteTag('S', 'K', 'S', 'L');
static constexpr SkFourByteTag kGLSL_Tag = SkSetFourByteTag('G', 'L', 'S', 'L');

void GrGLProgramBuilder::storeShaderInCache(const SkSL::Program::Inputs& inputs, GrGLuint programID,
                                            const SkSL::String shaders[], bool isSkSL) {
    if (!this->gpu()->getContext()->priv().getPersistentCache()) {
        return;
    }
    sk_sp<SkData> key = SkData::MakeWithoutCopy(desc()->asKey(), desc()->keyLength());
    if (fGpu->glCaps().programBinarySupport()) {
        // binary cache
        GrGLsizei length = 0;
        GL_CALL(GetProgramiv(programID, GL_PROGRAM_BINARY_LENGTH, &length));
        if (length > 0) {
            SkWriter32 writer;
            writer.writePad(&inputs, sizeof(inputs));
            writer.write32(length);

            void* binary = writer.reservePad(length);
            GrGLenum binaryFormat;
            GL_CALL(GetProgramBinary(programID, length, &length, &binaryFormat, binary));
            writer.write32(binaryFormat);

            auto data = writer.snapshotAsData();
            this->gpu()->getContext()->priv().getPersistentCache()->store(*key, *data);
        }
    } else {
        // source cache
        auto data = GrPersistentCacheUtils::PackCachedShaders(isSkSL ? kSKSL_Tag : kGLSL_Tag,
                                                              shaders, &inputs, 1);
        this->gpu()->getContext()->priv().getPersistentCache()->store(*key, *data);
    }
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
        this->gpu()->glCaps().programParameterSupport() &&
        this->gpu()->getContext()->priv().getPersistentCache()) {
        GL_CALL(ProgramParameteri(programID, GR_GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GR_GL_TRUE));
    }

    this->finalizeShaders();

    // compile shaders and bind attributes / uniforms
    auto errorHandler = this->gpu()->getContext()->priv().getShaderErrorHandler();
    const GrPrimitiveProcessor& primProc = this->primitiveProcessor();
    SkSL::Program::Settings settings;
    settings.fCaps = this->gpu()->glCaps().shaderCaps();
    settings.fFlipY = this->origin() != kTopLeft_GrSurfaceOrigin;
    settings.fSharpenTextures =
                    this->gpu()->getContext()->priv().options().fSharpenMipmappedTextures;
    settings.fFragColorIsInOut = this->fragColorIsInOut();

    SkSL::Program::Inputs inputs;
    SkTDArray<GrGLuint> shadersToDelete;
    // Calling GetProgramiv is expensive in Chromium. Assume success in release builds.
    bool checkLinked = kChromium_GrGLDriver != fGpu->ctxInfo().driver();
#ifdef SK_DEBUG
    checkLinked = true;
#endif
    bool cached = fCached.get() != nullptr;
    SkSL::String glsl[kGrShaderTypeCount];
    SkSL::String* sksl[kGrShaderTypeCount] = {
        &fVS.fCompilerString,
        &fGS.fCompilerString,
        &fFS.fCompilerString,
    };
#if GR_TEST_UTILS
    SkSL::String cached_sksl[kGrShaderTypeCount];
#endif
    if (cached) {
        if (fGpu->glCaps().programBinarySupport()) {
            // binary cache hit, just hand the binary to GL
            SkReader32 reader(fCached->data(), fCached->size());
            reader.read(&inputs, sizeof(inputs));
            GrGLsizei length = reader.readInt();
            const void* binary = reader.skip(length);
            GrGLenum binaryFormat = reader.readU32();
            GrGLClearErr(this->gpu()->glInterface());
            GR_GL_CALL_NOERRCHECK(this->gpu()->glInterface(),
                                  ProgramBinary(programID, binaryFormat, const_cast<void*>(binary),
                                                length));
            if (GR_GL_GET_ERROR(this->gpu()->glInterface()) == GR_GL_NO_ERROR) {
                if (checkLinked) {
                    cached = this->checkLinkStatus(programID, errorHandler, nullptr, nullptr);
                }
                if (cached) {
                    this->addInputVars(inputs);
                    this->computeCountsAndStrides(programID, primProc, false);
                }
            } else {
                cached = false;
            }
#if GR_TEST_UTILS
        } else if (fGpu->getContext()->priv().options().fCacheSKSL) {
            // Only switch to the stored SkSL if it unpacks correctly
            if (kSKSL_Tag == GrPersistentCacheUtils::UnpackCachedShaders(fCached.get(),
                                                                         cached_sksl,
                                                                         &inputs, 1)) {
                for (int i = 0; i < kGrShaderTypeCount; ++i) {
                    sksl[i] = &cached_sksl[i];
                }
            }
#endif
        } else {
            // source cache hit, we don't need to compile the SkSL->GLSL
            // It's unlikely, but if we get the wrong kind of shader back, don't use the strings
            if (kGLSL_Tag != GrPersistentCacheUtils::UnpackCachedShaders(fCached.get(),
                                                                         glsl,
                                                                         &inputs, 1)) {
                for (int i = 0; i < kGrShaderTypeCount; ++i) {
                    glsl[i].clear();
                }
            }
        }
    }
    if (!cached || !fGpu->glCaps().programBinarySupport()) {
        // either a cache miss, or we can't store binaries in the cache
        if (glsl[kFragment_GrShaderType].empty()) {
            // Don't have cached GLSL, need to compile SkSL->GLSL
            if (fFS.fForceHighPrecision) {
                settings.fForceHighPrecision = true;
            }
            std::unique_ptr<SkSL::Program> fs = GrSkSLtoGLSL(gpu()->glContext(),
                                                             SkSL::Program::kFragment_Kind,
                                                             *sksl[kFragment_GrShaderType],
                                                             settings,
                                                             &glsl[kFragment_GrShaderType],
                                                             errorHandler);
            if (!fs) {
                this->cleanupProgram(programID, shadersToDelete);
                return nullptr;
            }
            inputs = fs->fInputs;
            this->addInputVars(inputs);
        } else {
            // we've pulled GLSL and inputs from the cache, but still need to do some setup
            this->addInputVars(inputs);
            this->computeCountsAndStrides(programID, primProc, false);
        }
        if (!this->compileAndAttachShaders(glsl[kFragment_GrShaderType], programID,
                                           GR_GL_FRAGMENT_SHADER, &shadersToDelete, inputs,
                                           errorHandler)) {
            this->cleanupProgram(programID, shadersToDelete);
            return nullptr;
        }

        if (glsl[kVertex_GrShaderType].empty()) {
            // Don't have cached GLSL, need to compile SkSL->GLSL
            std::unique_ptr<SkSL::Program> vs = GrSkSLtoGLSL(gpu()->glContext(),
                                                             SkSL::Program::kVertex_Kind,
                                                             *sksl[kVertex_GrShaderType],
                                                             settings,
                                                             &glsl[kVertex_GrShaderType],
                                                             errorHandler);
            if (!vs) {
                this->cleanupProgram(programID, shadersToDelete);
                return nullptr;
            }
        }
        if (!this->compileAndAttachShaders(glsl[kVertex_GrShaderType], programID,
                                           GR_GL_VERTEX_SHADER, &shadersToDelete, inputs,
                                           errorHandler)) {
            this->cleanupProgram(programID, shadersToDelete);
            return nullptr;
        }

        // NVPR actually requires a vertex shader to compile
        bool useNvpr = primProc.isPathRendering();
        if (!useNvpr) {
            this->computeCountsAndStrides(programID, primProc, true);
        }

        if (primProc.willUseGeoShader()) {
            if (glsl[kGeometry_GrShaderType].empty()) {
                // Don't have cached GLSL, need to compile SkSL->GLSL
                std::unique_ptr<SkSL::Program> gs;
                gs = GrSkSLtoGLSL(gpu()->glContext(),
                                  SkSL::Program::kGeometry_Kind,
                                  *sksl[kGeometry_GrShaderType],
                                  settings,
                                  &glsl[kGeometry_GrShaderType],
                                  errorHandler);
                if (!gs) {
                    this->cleanupProgram(programID, shadersToDelete);
                    return nullptr;
                }
            }
            if (!this->compileAndAttachShaders(glsl[kGeometry_GrShaderType], programID,
                                               GR_GL_GEOMETRY_SHADER, &shadersToDelete, inputs,
                                               errorHandler)) {
                this->cleanupProgram(programID, shadersToDelete);
                return nullptr;
            }
        }
        this->bindProgramResourceLocations(programID);

        GL_CALL(LinkProgram(programID));
        if (checkLinked) {
            if (!this->checkLinkStatus(programID, errorHandler, sksl, glsl)) {
                GL_CALL(DeleteProgram(programID));
                return nullptr;
            }
        }
    }
    this->resolveProgramResourceLocations(programID);

    this->cleanupShaders(shadersToDelete);
    if (!cached) {
        bool isSkSL = false;
#if GR_TEST_UTILS
        if (fGpu->getContext()->priv().options().fCacheSKSL) {
            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                glsl[i] = GrShaderUtils::PrettyPrint(*sksl[i]);
            }
            isSkSL = true;
        }
#endif
        this->storeShaderInCache(inputs, programID, glsl, isSkSL);
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

bool GrGLProgramBuilder::checkLinkStatus(GrGLuint programID,
                                         GrContextOptions::ShaderErrorHandler* errorHandler,
                                         SkSL::String* sksl[], const SkSL::String glsl[]) {
    GrGLint linked = GR_GL_INIT_ZERO;
    GL_CALL(GetProgramiv(programID, GR_GL_LINK_STATUS, &linked));
    if (!linked) {
        SkSL::String allShaders;
        if (sksl) {
            allShaders.appendf("// Vertex SKSL\n%s\n", sksl[kVertex_GrShaderType]->c_str());
            if (!sksl[kGeometry_GrShaderType]->empty()) {
                allShaders.appendf("// Geometry SKSL\n%s\n", sksl[kGeometry_GrShaderType]->c_str());
            }
            allShaders.appendf("// Fragment SKSL\n%s\n", sksl[kFragment_GrShaderType]->c_str());
        }
        if (glsl) {
            allShaders.appendf("// Vertex GLSL\n%s\n", glsl[kVertex_GrShaderType].c_str());
            if (!glsl[kGeometry_GrShaderType].empty()) {
                allShaders.appendf("// Geometry GLSL\n%s\n", glsl[kGeometry_GrShaderType].c_str());
            }
            allShaders.appendf("// Fragment GLSL\n%s\n", glsl[kFragment_GrShaderType].c_str());
        }
        GrGLint infoLen = GR_GL_INIT_ZERO;
        GL_CALL(GetProgramiv(programID, GR_GL_INFO_LOG_LENGTH, &infoLen));
        SkAutoMalloc log(sizeof(char)*(infoLen+1));  // outside if for debugger
        if (infoLen > 0) {
            // retrieve length even though we don't need it to workaround
            // bug in chrome cmd buffer param validation.
            GrGLsizei length = GR_GL_INIT_ZERO;
            GL_CALL(GetProgramInfoLog(programID, infoLen+1, &length, (char*)log.get()));
        }
        errorHandler->compileError(allShaders.c_str(), infoLen > 0 ? (const char*)log.get() : "");
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
