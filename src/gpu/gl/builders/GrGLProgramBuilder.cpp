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

static void cleanup_shaders(GrGLGpu* gpu, const SkTDArray<GrGLuint>& shaderIDs) {
    for (int i = 0; i < shaderIDs.count(); ++i) {
        GR_GL_CALL(gpu->glInterface(), DeleteShader(shaderIDs[i]));
    }
}

static void cleanup_program(GrGLGpu* gpu, GrGLuint programID,
                            const SkTDArray<GrGLuint>& shaderIDs) {
    GR_GL_CALL(gpu->glInterface(), DeleteProgram(programID));
    cleanup_shaders(gpu, shaderIDs);
}

GrGLProgram* GrGLProgramBuilder::CreateProgram(GrRenderTarget* renderTarget, GrSurfaceOrigin origin,
                                               const GrPrimitiveProcessor& primProc,
                                               const GrTextureProxy* const primProcProxies[],
                                               const GrPipeline& pipeline,
                                               GrProgramDesc* desc,
                                               GrGLGpu* gpu,
                                               const GrGLPrecompiledProgram* precompiledProgram) {
    SkASSERT(!pipeline.isBad());

    ATRACE_ANDROID_FRAMEWORK("Shader Compile");
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrGLProgramBuilder builder(gpu, renderTarget, origin,
                               pipeline, primProc, primProcProxies, desc);

    auto persistentCache = gpu->getContext()->priv().getPersistentCache();
    if (persistentCache && !precompiledProgram) {
        sk_sp<SkData> key = SkData::MakeWithoutCopy(desc->asKey(), desc->keyLength());
        builder.fCached = persistentCache->load(*key);
        // the eventual end goal is to completely skip emitAndInstallProcs on a cache hit, but it's
        // doing necessary setup in addition to generating the SkSL code. Currently we are only able
        // to skip the SkSL->GLSL step on a cache hit.
    }
    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }
    return builder.finalize(precompiledProgram);
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
static constexpr SkFourByteTag kGLPB_Tag = SkSetFourByteTag('G', 'L', 'P', 'B');

void GrGLProgramBuilder::storeShaderInCache(const SkSL::Program::Inputs& inputs, GrGLuint programID,
                                            const SkSL::String shaders[], bool isSkSL,
                                            const SkSL::Program::Settings& settings) {
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
            writer.write32(kGLPB_Tag);

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
                                                              shaders, &inputs, 1, &settings);
        this->gpu()->getContext()->priv().getPersistentCache()->store(*key, *data);
    }
}

GrGLProgram* GrGLProgramBuilder::finalize(const GrGLPrecompiledProgram* precompiledProgram) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    // verify we can get a program id
    GrGLuint programID;
    if (precompiledProgram) {
        programID = precompiledProgram->fProgramID;
    } else {
        GL_CALL_RET(programID, CreateProgram());
    }
    if (0 == programID) {
        return nullptr;
    }

    if (this->gpu()->glCaps().programBinarySupport() &&
        this->gpu()->glCaps().programParameterSupport() &&
        this->gpu()->getContext()->priv().getPersistentCache() &&
        !precompiledProgram) {
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
    bool usedProgramBinaries = false;
    SkSL::String glsl[kGrShaderTypeCount];
    SkSL::String* sksl[kGrShaderTypeCount] = {
        &fVS.fCompilerString,
        &fGS.fCompilerString,
        &fFS.fCompilerString,
    };
    SkSL::String cached_sksl[kGrShaderTypeCount];
    if (precompiledProgram) {
        // This is very similar to when we get program binaries. We even set that flag, as it's
        // used to prevent other compile work later, and to force re-querying uniform locations.
        this->addInputVars(precompiledProgram->fInputs);
        this->computeCountsAndStrides(programID, primProc, false);
        usedProgramBinaries = true;
    } else if (cached) {
        SkReader32 reader(fCached->data(), fCached->size());
        SkFourByteTag shaderType = reader.readU32();

        switch (shaderType) {
            case kGLPB_Tag: {
                // Program binary cache hit. We may opt not to use this if we don't trust program
                // binaries on this driver
                if (!fGpu->glCaps().programBinarySupport()) {
                    cached = false;
                    break;
                }
                reader.read(&inputs, sizeof(inputs));
                GrGLsizei length = reader.readInt();
                const void* binary = reader.skip(length);
                GrGLenum binaryFormat = reader.readU32();
                GrGLClearErr(this->gpu()->glInterface());
                GR_GL_CALL_NOERRCHECK(this->gpu()->glInterface(),
                                      ProgramBinary(programID, binaryFormat,
                                                    const_cast<void*>(binary), length));
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
                usedProgramBinaries = cached;
                break;
            }

            case kGLSL_Tag:
                // Source cache hit, we don't need to compile the SkSL->GLSL
                GrPersistentCacheUtils::UnpackCachedShaders(&reader, glsl, &inputs, 1);
                break;

            case kSKSL_Tag:
                // SkSL cache hit, this should only happen in tools overriding the generated SkSL
                GrPersistentCacheUtils::UnpackCachedShaders(&reader, cached_sksl, &inputs, 1);
                for (int i = 0; i < kGrShaderTypeCount; ++i) {
                    sksl[i] = &cached_sksl[i];
                }
                break;
        }
    }
    if (!usedProgramBinaries) {
        // either a cache miss, or we got something other than binaries from the cache
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
                cleanup_program(fGpu, programID, shadersToDelete);
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
                                           GR_GL_FRAGMENT_SHADER, &shadersToDelete, errorHandler)) {
            cleanup_program(fGpu, programID, shadersToDelete);
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
                cleanup_program(fGpu, programID, shadersToDelete);
                return nullptr;
            }
        }
        if (!this->compileAndAttachShaders(glsl[kVertex_GrShaderType], programID,
                                           GR_GL_VERTEX_SHADER, &shadersToDelete, errorHandler)) {
            cleanup_program(fGpu, programID, shadersToDelete);
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
                    cleanup_program(fGpu, programID, shadersToDelete);
                    return nullptr;
                }
            }
            if (!this->compileAndAttachShaders(glsl[kGeometry_GrShaderType], programID,
                                               GR_GL_GEOMETRY_SHADER, &shadersToDelete,
                                               errorHandler)) {
                cleanup_program(fGpu, programID, shadersToDelete);
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
    this->resolveProgramResourceLocations(programID, usedProgramBinaries);

    cleanup_shaders(fGpu, shadersToDelete);

    // With ANGLE, we can't cache path-rendering programs. We use ProgramPathFragmentInputGen,
    // and ANGLE's deserialized program state doesn't restore enough state to handle that.
    // The native NVIDIA drivers do, but this is such an edge case that it's easier to just
    // black-list caching these programs in all cases. See: anglebug.com/3619
    // We also can't cache SkSL or GLSL if we were given a precompiled program, but there's not
    // much point in doing so.
    if (!cached && !primProc.isPathRendering() && !precompiledProgram) {
        bool isSkSL = false;
        if (fGpu->getContext()->priv().options().fShaderCacheStrategy ==
                GrContextOptions::ShaderCacheStrategy::kSkSL) {
            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                glsl[i] = GrShaderUtils::PrettyPrint(*sksl[i]);
            }
            isSkSL = true;
        }
        this->storeShaderInCache(inputs, programID, glsl, isSkSL, settings);
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

void GrGLProgramBuilder::resolveProgramResourceLocations(GrGLuint programID, bool force) {
    fUniformHandler.getUniformLocations(programID, fGpu->glCaps(), force);

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

bool GrGLProgramBuilder::PrecompileProgram(GrGLPrecompiledProgram* precompiledProgram,
                                           GrGLGpu* gpu,
                                           const SkData& cachedData) {
    SkReader32 reader(cachedData.data(), cachedData.size());
    SkFourByteTag shaderType = reader.readU32();
    if (shaderType != kSKSL_Tag) {
        // TODO: Support GLSL, and maybe even program binaries, too?
        return false;
    }

    const GrGLInterface* gl = gpu->glInterface();
    auto errorHandler = gpu->getContext()->priv().getShaderErrorHandler();
    GrGLuint programID;
    GR_GL_CALL_RET(gl, programID, CreateProgram());
    if (0 == programID) {
        return false;
    }

    SkTDArray<GrGLuint> shadersToDelete;

    SkSL::Program::Settings settings;
    settings.fCaps = gpu->glCaps().shaderCaps();
    settings.fSharpenTextures = gpu->getContext()->priv().options().fSharpenMipmappedTextures;

    SkSL::String shaders[kGrShaderTypeCount];
    SkSL::Program::Inputs inputs;
    GrPersistentCacheUtils::UnpackCachedShaders(&reader, shaders, &inputs, 1, &settings);

    auto compileShader = [&](SkSL::Program::Kind kind, const SkSL::String& sksl, GrGLenum type) {
        SkSL::String glsl;
        auto program = GrSkSLtoGLSL(gpu->glContext(), kind, sksl, settings, &glsl, errorHandler);
        if (!program) {
            return false;
        }

        if (GrGLuint shaderID = GrGLCompileAndAttachShader(gpu->glContext(), programID, type, glsl,
                                                           gpu->stats(), errorHandler)) {
            shadersToDelete.push_back(shaderID);
            return true;
        } else {
            return false;
        }
    };

    if (!compileShader(SkSL::Program::kFragment_Kind,
                       shaders[kFragment_GrShaderType],
                       GR_GL_FRAGMENT_SHADER) ||
        !compileShader(SkSL::Program::kVertex_Kind,
                       shaders[kVertex_GrShaderType],
                       GR_GL_VERTEX_SHADER) ||
        (!shaders[kGeometry_GrShaderType].empty() &&
         !compileShader(SkSL::Program::kGeometry_Kind,
                       shaders[kGeometry_GrShaderType],
                       GR_GL_GEOMETRY_SHADER))) {
        cleanup_program(gpu, programID, shadersToDelete);
        return false;
    }

    GR_GL_CALL(gpu->glInterface(), LinkProgram(programID));
    GrGLint linked = GR_GL_INIT_ZERO;
    GR_GL_CALL(gpu->glInterface(), GetProgramiv(programID, GR_GL_LINK_STATUS, &linked));
    if (!linked) {
        cleanup_program(gpu, programID, shadersToDelete);
        return false;
    }

    cleanup_shaders(gpu, shadersToDelete);

    precompiledProgram->fProgramID = programID;
    precompiledProgram->fInputs = inputs;
    return true;
}
