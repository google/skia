/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/gl/builders/GrGLProgramBuilder.h"

#include "include/core/SkFourByteTag.h"
#include "include/core/SkTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLFunctions.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkWriteBuffer.h"
#include "src/gpu/ganesh/GrAutoLocaleSetter.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrPersistentCacheUtils.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h"
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLProgram.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "src/gpu/ganesh/gl/builders/GrGLShaderStringBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/utils/SkShaderUtils.h"

#include <cstdint>
#include <memory>
#include <utility>

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

static void cleanup_shaders(GrGLGpu* gpu, const SkTDArray<GrGLuint>& shaderIDs) {
    for (int i = 0; i < shaderIDs.size(); ++i) {
        GR_GL_CALL(gpu->glInterface(), DeleteShader(shaderIDs[i]));
    }
}

static void cleanup_program(GrGLGpu* gpu, GrGLuint programID,
                            const SkTDArray<GrGLuint>& shaderIDs) {
    GR_GL_CALL(gpu->glInterface(), DeleteProgram(programID));
    cleanup_shaders(gpu, shaderIDs);
}

sk_sp<GrGLProgram> GrGLProgramBuilder::CreateProgram(
                                               GrDirectContext* dContext,
                                               const GrProgramDesc& desc,
                                               const GrProgramInfo& programInfo,
                                               const GrGLPrecompiledProgram* precompiledProgram) {
    TRACE_EVENT0_ALWAYS("skia.shaders", "shader_compile");
    GrAutoLocaleSetter als("C");

    GrGLGpu* glGpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrGLProgramBuilder builder(glGpu, desc, programInfo);

    auto persistentCache = dContext->priv().getPersistentCache();
    if (persistentCache && !precompiledProgram) {
        sk_sp<SkData> key = SkData::MakeWithoutCopy(desc.asKey(), desc.keyLength());
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
                                       const GrProgramDesc& desc,
                                       const GrProgramInfo& programInfo)
        : INHERITED(desc, programInfo)
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

bool GrGLProgramBuilder::compileAndAttachShaders(const std::string& glsl,
                                                 GrGLuint programId,
                                                 GrGLenum type,
                                                 SkTDArray<GrGLuint>* shaderIds,
                                                 bool shaderWasCached,
                                                 GrContextOptions::ShaderErrorHandler* errHandler) {
    GrGLGpu* gpu = this->gpu();
    GrGLuint shaderId = GrGLCompileAndAttachShader(gpu->glContext(),
                                                   programId,
                                                   type,
                                                   glsl,
                                                   shaderWasCached,
                                                   gpu->pipelineBuilder()->stats(),
                                                   errHandler);
    if (!shaderId) {
        return false;
    }

    *shaderIds->append() = shaderId;
    return true;
}

void GrGLProgramBuilder::computeCountsAndStrides(GrGLuint programID,
                                                 const GrGeometryProcessor& geomProc,
                                                 bool bindAttribLocations) {
    fVertexAttributeCnt = geomProc.numVertexAttributes();
    fInstanceAttributeCnt = geomProc.numInstanceAttributes();
    fAttributes = std::make_unique<GrGLProgram::Attribute[]>(
            fVertexAttributeCnt + fInstanceAttributeCnt);
    auto addAttr = [&](int i, const auto& a) {
        fAttributes[i].fCPUType = a.cpuType();
        fAttributes[i].fGPUType = a.gpuType();
        fAttributes[i].fOffset = *a.offset();
        fAttributes[i].fLocation = i;
        if (bindAttribLocations) {
            GL_CALL(BindAttribLocation(programID, i, a.name()));
        }
    };
    fVertexStride = geomProc.vertexStride();
    int i = 0;
    for (auto attr : geomProc.vertexAttributes()) {
        addAttr(i++, attr);
    }
    fInstanceStride = geomProc.instanceStride();
    for (auto attr : geomProc.instanceAttributes()) {
        addAttr(i++, attr);
    }
    SkASSERT(fInstanceStride == geomProc.instanceStride());
}

void GrGLProgramBuilder::addInputVars(const SkSL::Program::Interface& interface) {
    uint8_t useRTFlip = interface.fRTFlipUniform;
    if (!this->gpu()->glCaps().shaderCaps()->fCanUseFragCoord) {
        useRTFlip &= ~SkSL::Program::Interface::kRTFlip_FragCoord;
    }

    if (useRTFlip != SkSL::Program::Interface::kRTFlip_None) {
        this->addRTFlipUniform(SKSL_RTFLIP_NAME);
    }
}

static constexpr SkFourByteTag kSKSL_Tag = SkSetFourByteTag('S', 'K', 'S', 'L');
static constexpr SkFourByteTag kGLSL_Tag = SkSetFourByteTag('G', 'L', 'S', 'L');
static constexpr SkFourByteTag kGLPB_Tag = SkSetFourByteTag('G', 'L', 'P', 'B');

void GrGLProgramBuilder::storeShaderInCache(const SkSL::Program::Interface& interface,
                                            GrGLuint programID,
                                            const std::string shaders[],
                                            bool isSkSL,
                                            SkSL::ProgramSettings* settings) {
    if (!this->gpu()->getContext()->priv().getPersistentCache()) {
        return;
    }
    sk_sp<SkData> key = SkData::MakeWithoutCopy(this->desc().asKey(), this->desc().keyLength());
    SkString description = GrProgramDesc::Describe(fProgramInfo, *fGpu->caps());
    if (fGpu->glCaps().programBinarySupport()) {
        // binary cache
        GrGLsizei length = 0;
        GL_CALL(GetProgramiv(programID, GL_PROGRAM_BINARY_LENGTH, &length));
        if (length > 0) {
            SkBinaryWriteBuffer writer({});
            writer.writeInt(GrPersistentCacheUtils::GetCurrentVersion());
            writer.writeUInt(kGLPB_Tag);

            writer.writePad32(&interface, sizeof(interface));

            SkAutoSMalloc<2048> binary(length);
            GrGLenum binaryFormat;
            GL_CALL(GetProgramBinary(programID, length, &length, &binaryFormat, binary.get()));

            writer.writeUInt(binaryFormat);
            writer.writeInt(length);
            writer.writePad32(binary.get(), length);

            auto data = writer.snapshotAsData();
            this->gpu()->getContext()->priv().getPersistentCache()->store(*key, *data, description);
        }
    } else {
        // source cache, plus metadata to allow for a complete precompile
        GrPersistentCacheUtils::ShaderMetadata meta;
        meta.fSettings = settings;
        meta.fHasSecondaryColorOutput = fFS.hasSecondaryOutput();
        for (auto attr : this->geometryProcessor().vertexAttributes()) {
            meta.fAttributeNames.emplace_back(attr.name());
        }
        for (auto attr : this->geometryProcessor().instanceAttributes()) {
            meta.fAttributeNames.emplace_back(attr.name());
        }

        auto data = GrPersistentCacheUtils::PackCachedShaders(isSkSL ? kSKSL_Tag : kGLSL_Tag,
                                                              shaders, &interface, 1, &meta);
        this->gpu()->getContext()->priv().getPersistentCache()->store(*key, *data, description);
    }
}

sk_sp<GrGLProgram> GrGLProgramBuilder::finalize(const GrGLPrecompiledProgram* precompiledProgram) {
    TRACE_EVENT0("skia.shaders", TRACE_FUNC);

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
    const GrGeometryProcessor& geomProc = this->geometryProcessor();
    SkSL::ProgramSettings settings;
    settings.fSharpenTextures =
            this->gpu()->getContext()->priv().options().fSharpenMipmappedTextures;
    settings.fFragColorIsInOut = this->fragColorIsInOut();

    SkSL::Program::Interface interface;
    SkTDArray<GrGLuint> shadersToDelete;

    bool cached = fCached.get() != nullptr;
    bool usedProgramBinaries = false;
    std::string glsl[kGrShaderTypeCount];
    const std::string* sksl[kGrShaderTypeCount] = {
        &fVS.fCompilerString,
        &fFS.fCompilerString,
    };
    std::string cached_sksl[kGrShaderTypeCount];
    if (precompiledProgram) {
        // This is very similar to when we get program binaries. We even set that flag, as it's
        // used to prevent other compile work later, and to force re-querying uniform locations.
        this->addInputVars(precompiledProgram->fInterface);
        this->computeCountsAndStrides(programID, geomProc, false);
        usedProgramBinaries = true;
    } else if (cached) {
        TRACE_EVENT0_ALWAYS("skia.shaders", "cache_hit");
        SkReadBuffer reader(fCached->data(), fCached->size());
        SkFourByteTag shaderType = GrPersistentCacheUtils::GetType(&reader);

        switch (shaderType) {
            case kGLPB_Tag: {
                // Program binary cache hit. We may opt not to use this if we don't trust program
                // binaries on this driver
                if (!fGpu->glCaps().programBinarySupport()) {
                    cached = false;
                    break;
                }
                reader.readPad32(&interface, sizeof(interface));
                GrGLenum binaryFormat = reader.readUInt();
                GrGLsizei length      = reader.readInt();
                const void* binary = reader.skip(length);
                if (!reader.isValid()) {
                    break;
                }
                if (length <= 0 || !fGpu->glCaps().programBinaryFormatIsValid(binaryFormat)) {
                    cached = false;
                    break;
                }
                GL_CALL(ProgramBinary(programID, binaryFormat, const_cast<void*>(binary), length));
                // Pass nullptr for the error handler. We don't want to treat this as a compile
                // failure (we can still recover by compiling the program from source, below).
                // Clients won't be directly notified, but they can infer this from the trace
                // events, and from the traffic to the persistent cache.
                cached = GrGLCheckLinkStatus(fGpu, programID, /*shaderWasCached=*/true,
                                             /*errorHandler=*/nullptr, nullptr, nullptr);
                if (cached) {
                    this->addInputVars(interface);
                    this->computeCountsAndStrides(programID, geomProc, false);
                }
                usedProgramBinaries = cached;
                break;
            }

            case kGLSL_Tag:
                // Source cache hit, we don't need to compile the SkSL->GLSL
                GrPersistentCacheUtils::UnpackCachedShaders(&reader, glsl, &interface, 1);
                break;

            case kSKSL_Tag:
                // SkSL cache hit, this should only happen in tools overriding the generated SkSL
                if (GrPersistentCacheUtils::UnpackCachedShaders(
                            &reader, cached_sksl, &interface, 1)) {
                    for (int i = 0; i < kGrShaderTypeCount; ++i) {
                        sksl[i] = &cached_sksl[i];
                    }
                }
                break;

            default:
                // We got something invalid, so pretend it wasn't there
                reader.validate(false);
                break;
        }
        if (!reader.isValid()) {
            cached = false;
        }
    }
    if (!usedProgramBinaries) {
        TRACE_EVENT0_ALWAYS("skia.shaders", "cache_miss");
        // Either a cache miss, or we got something other than binaries from the cache

        /*
           Fragment Shader
        */
        if (glsl[kFragment_GrShaderType].empty()) {
            // Don't have cached GLSL, need to compile SkSL->GLSL
            if (fFS.fForceHighPrecision) {
                settings.fForceHighPrecision = true;
            }
            if (!skgpu::SkSLToGLSL(this->gpu()->caps()->shaderCaps(),
                                   *sksl[kFragment_GrShaderType],
                                   SkSL::ProgramKind::kFragment,
                                   settings,
                                   &glsl[kFragment_GrShaderType],
                                   &interface,
                                   errorHandler)) {
                cleanup_program(fGpu, programID, shadersToDelete);
                return nullptr;
            }
        }

        this->addInputVars(interface);
        if (!this->compileAndAttachShaders(glsl[kFragment_GrShaderType],
                                           programID,
                                           GR_GL_FRAGMENT_SHADER,
                                           &shadersToDelete,
                                           cached,
                                           errorHandler)) {
            cleanup_program(fGpu, programID, shadersToDelete);
            return nullptr;
        }

        /*
           Vertex Shader
        */
        if (glsl[kVertex_GrShaderType].empty()) {
            // Don't have cached GLSL, need to compile SkSL->GLSL
            SkSL::Program::Interface unusedInterface;
            if (!skgpu::SkSLToGLSL(this->gpu()->caps()->shaderCaps(),
                                   *sksl[kVertex_GrShaderType],
                                   SkSL::ProgramKind::kVertex,
                                   settings,
                                   &glsl[kVertex_GrShaderType],
                                   &unusedInterface,
                                   errorHandler)) {
                cleanup_program(fGpu, programID, shadersToDelete);
                return nullptr;
            }
        }
        if (!this->compileAndAttachShaders(glsl[kVertex_GrShaderType],
                                           programID,
                                           GR_GL_VERTEX_SHADER,
                                           &shadersToDelete,
                                           cached,
                                           errorHandler)) {
            cleanup_program(fGpu, programID, shadersToDelete);
            return nullptr;
        }

        // This also binds vertex attribute locations.
        this->computeCountsAndStrides(programID, geomProc, true);

        this->bindProgramResourceLocations(programID);

        {
            TRACE_EVENT0_ALWAYS("skia.shaders", "driver_link_program");
            GL_CALL(LinkProgram(programID));
            if (!GrGLCheckLinkStatus(fGpu, programID, cached, errorHandler, sksl, glsl)) {
                cleanup_program(fGpu, programID, shadersToDelete);
                return nullptr;
            }
        }
    }
    this->resolveProgramResourceLocations(programID, usedProgramBinaries);

    cleanup_shaders(fGpu, shadersToDelete);

    // We can't cache SkSL or GLSL if we were given a precompiled program, but there's not
    // much point in doing so.
    if (!cached && !precompiledProgram) {
        bool isSkSL = false;
        if (fGpu->getContext()->priv().options().fShaderCacheStrategy ==
                GrContextOptions::ShaderCacheStrategy::kSkSL) {
            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                glsl[i] = SkShaderUtils::PrettyPrint(*sksl[i]);
            }
            isSkSL = true;
        }
        this->storeShaderInCache(interface, programID, glsl, isSkSL, &settings);
    }
    return this->createProgram(programID);
}

void GrGLProgramBuilder::bindProgramResourceLocations(GrGLuint programID) {
    fUniformHandler.bindUniformLocations(programID, fGpu->glCaps());

    const GrGLCaps& caps = this->gpu()->glCaps();
    if (caps.bindFragDataLocationSupport()) {
        SkASSERT(caps.shaderCaps()->mustDeclareFragmentShaderOutput());
        GL_CALL(BindFragDataLocation(programID, 0,
                                     GrGLSLFragmentShaderBuilder::DeclaredColorOutputName()));
        if (fFS.hasSecondaryOutput()) {
            GL_CALL(BindFragDataLocationIndexed(programID, 0, 1,
                                  GrGLSLFragmentShaderBuilder::DeclaredSecondaryColorOutputName()));
        }
    }
}

void GrGLProgramBuilder::resolveProgramResourceLocations(GrGLuint programID, bool force) {
    fUniformHandler.getUniformLocations(programID, fGpu->glCaps(), force);
}

sk_sp<GrGLProgram> GrGLProgramBuilder::createProgram(GrGLuint programID) {
    return GrGLProgram::Make(fGpu,
                             fUniformHandles,
                             programID,
                             fUniformHandler.fUniforms,
                             fUniformHandler.fSamplers,
                             std::move(fGPImpl),
                             std::move(fXPImpl),
                             std::move(fFPImpls),
                             std::move(fAttributes),
                             fVertexAttributeCnt,
                             fInstanceAttributeCnt,
                             fVertexStride,
                             fInstanceStride);
}

bool GrGLProgramBuilder::PrecompileProgram(GrDirectContext* dContext,
                                           GrGLPrecompiledProgram* precompiledProgram,
                                           const SkData& cachedData) {
    SkReadBuffer reader(cachedData.data(), cachedData.size());
    SkFourByteTag shaderType = GrPersistentCacheUtils::GetType(&reader);
    if (shaderType != kSKSL_Tag) {
        // TODO: Support GLSL, and maybe even program binaries, too?
        return false;
    }

    GrGLGpu* glGpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());

    const GrGLInterface* gl = glGpu->glInterface();
    auto errorHandler = dContext->priv().getShaderErrorHandler();

    SkSL::ProgramSettings settings;
    settings.fSharpenTextures = dContext->priv().options().fSharpenMipmappedTextures;
    GrPersistentCacheUtils::ShaderMetadata meta;
    meta.fSettings = &settings;

    std::string shaders[kGrShaderTypeCount];
    SkSL::Program::Interface interface;
    if (!GrPersistentCacheUtils::UnpackCachedShaders(&reader, shaders, &interface, 1, &meta)) {
        return false;
    }

    GrGLuint programID;
    GR_GL_CALL_RET(gl, programID, CreateProgram());
    if (0 == programID) {
        return false;
    }

    SkTDArray<GrGLuint> shadersToDelete;

    auto compileShader = [&](SkSL::ProgramKind kind, const std::string& sksl, GrGLenum type) {
        std::string glsl;
        SkSL::Program::Interface unusedInterface;
        if (!skgpu::SkSLToGLSL(glGpu->caps()->shaderCaps(),
                               sksl,
                               kind,
                               settings,
                               &glsl,
                               &unusedInterface,
                               errorHandler)) {
            return false;
        }

        if (GrGLuint shaderID = GrGLCompileAndAttachShader(glGpu->glContext(),
                                                           programID,
                                                           type,
                                                           glsl,
                                                           /*shaderWasCached=*/false,
                                                           glGpu->pipelineBuilder()->stats(),
                                                           errorHandler)) {
            shadersToDelete.push_back(shaderID);
            return true;
        } else {
            return false;
        }
    };

    if (!compileShader(SkSL::ProgramKind::kFragment,
                       shaders[kFragment_GrShaderType],
                       GR_GL_FRAGMENT_SHADER) ||
        !compileShader(SkSL::ProgramKind::kVertex,
                       shaders[kVertex_GrShaderType],
                       GR_GL_VERTEX_SHADER)) {
        cleanup_program(glGpu, programID, shadersToDelete);
        return false;
    }

    for (int i = 0; i < meta.fAttributeNames.size(); ++i) {
        GR_GL_CALL(glGpu->glInterface(), BindAttribLocation(programID, i,
                                                          meta.fAttributeNames[i].c_str()));
    }

    const GrGLCaps& caps = glGpu->glCaps();
    if (caps.bindFragDataLocationSupport()) {
        SkASSERT(caps.shaderCaps()->mustDeclareFragmentShaderOutput());
        GR_GL_CALL(glGpu->glInterface(),
                   BindFragDataLocation(programID, 0,
                                        GrGLSLFragmentShaderBuilder::DeclaredColorOutputName()));

        if (meta.fHasSecondaryColorOutput) {
            GR_GL_CALL(glGpu->glInterface(),
                       BindFragDataLocationIndexed(programID, 0, 1,
                                  GrGLSLFragmentShaderBuilder::DeclaredSecondaryColorOutputName()));
        }
    }

    GR_GL_CALL(glGpu->glInterface(), LinkProgram(programID));
    GrGLint linked = GR_GL_INIT_ZERO;
    GR_GL_CALL(glGpu->glInterface(), GetProgramiv(programID, GR_GL_LINK_STATUS, &linked));
    if (!linked) {
        cleanup_program(glGpu, programID, shadersToDelete);
        return false;
    }

    cleanup_shaders(glGpu, shadersToDelete);

    precompiledProgram->fProgramID = programID;
    precompiledProgram->fInterface = interface;
    return true;
}
