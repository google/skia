/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

#include <memory>

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/dsl/priv/DSLFPs.h"

const int GrGLSLProgramBuilder::kVarsPerBlock = 8;

GrGLSLProgramBuilder::GrGLSLProgramBuilder(GrRenderTarget* renderTarget,
                                           const GrProgramDesc& desc,
                                           const GrProgramInfo& programInfo)
        : fVS(this)
        , fGS(this)
        , fFS(this)
        , fStageIndex(-1)
        , fRenderTarget(renderTarget)
        , fDesc(desc)
        , fProgramInfo(programInfo)
        , fGeometryProcessor(nullptr)
        , fXferProcessor(nullptr)
        , fNumFragmentSamplers(0) {}

GrGLSLProgramBuilder::~GrGLSLProgramBuilder() = default;

void GrGLSLProgramBuilder::addFeature(GrShaderFlags shaders,
                                      uint32_t featureBit,
                                      const char* extensionName) {
    if (shaders & kVertex_GrShaderFlag) {
        fVS.addFeature(featureBit, extensionName);
    }
    if (shaders & kGeometry_GrShaderFlag) {
        SkASSERT(this->primitiveProcessor().willUseGeoShader());
        fGS.addFeature(featureBit, extensionName);
    }
    if (shaders & kFragment_GrShaderFlag) {
        fFS.addFeature(featureBit, extensionName);
    }
}

bool GrGLSLProgramBuilder::emitAndInstallProcs() {
    // First we loop over all of the installed processors and collect coord transforms.  These will
    // be sent to the GrGLSLPrimitiveProcessor in its emitCode function
    SkSL::dsl::Start(this->shaderCompiler());
    SkString inputColor;
    SkString inputCoverage;
    this->emitAndInstallPrimProc(&inputColor, &inputCoverage);
    this->emitAndInstallFragProcs(&inputColor, &inputCoverage);
    this->emitAndInstallXferProc(inputColor, inputCoverage);
    fGeometryProcessor->emitTransformCode(&fVS, this->uniformHandler());
    SkSL::dsl::End();

    return this->checkSamplerCounts();
}

void GrGLSLProgramBuilder::emitAndInstallPrimProc(SkString* outputColor, SkString* outputCoverage) {
    const GrPrimitiveProcessor& proc = this->primitiveProcessor();

    // Program builders have a bit of state we need to clear with each effect
    AutoStageAdvance adv(this);
    this->nameExpression(outputColor, "outputColor");
    this->nameExpression(outputCoverage, "outputCoverage");

    SkASSERT(!fUniformHandles.fRTAdjustmentUni.isValid());
    GrShaderFlags rtAdjustVisibility;
    if (proc.willUseGeoShader()) {
        rtAdjustVisibility = kGeometry_GrShaderFlag;
    } else if (proc.willUseTessellationShaders()) {
        rtAdjustVisibility = kTessEvaluation_GrShaderFlag;
    } else {
        rtAdjustVisibility = kVertex_GrShaderFlag;
    }
    fUniformHandles.fRTAdjustmentUni = this->uniformHandler()->addUniform(
            nullptr, rtAdjustVisibility, kFloat4_GrSLType, SkSL::Compiler::RTADJUST_NAME);

    fFS.codeAppendf("// Stage %d, %s\n", fStageIndex, proc.name());
    fVS.codeAppendf("// Primitive Processor %s\n", proc.name());

    SkASSERT(!fGeometryProcessor);
    fGeometryProcessor.reset(proc.createGLSLInstance(*this->shaderCaps()));

    SkAutoSTMalloc<4, SamplerHandle> texSamplers(proc.numTextureSamplers());
    for (int i = 0; i < proc.numTextureSamplers(); ++i) {
        SkString name;
        name.printf("TextureSampler_%d", i);
        const auto& sampler = proc.textureSampler(i);
        texSamplers[i] = this->emitSampler(proc.textureSampler(i).backendFormat(),
                                           sampler.samplerState(),
                                           sampler.swizzle(),
                                           name.c_str());
    }

    GrGLSLPrimitiveProcessor::FPCoordTransformHandler transformHandler(this->pipeline(),
                                                                       &fTransformedCoordVars);
    GrGLSLGeometryProcessor::EmitArgs args(&fVS,
                                           proc.willUseGeoShader() ? &fGS : nullptr,
                                           &fFS,
                                           this->varyingHandler(),
                                           this->uniformHandler(),
                                           this->shaderCaps(),
                                           proc,
                                           outputColor->c_str(),
                                           outputCoverage->c_str(),
                                           texSamplers.get(),
                                           &transformHandler);
    fGeometryProcessor->emitCode(args);

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    SkDEBUGCODE(verify(proc);)
}

void GrGLSLProgramBuilder::emitAndInstallFragProcs(SkString* color, SkString* coverage) {
    int transformedCoordVarsIdx = 0;
    int fpCount = this->pipeline().numFragmentProcessors();
    SkASSERT(fFPImpls.empty());
    fFPImpls.reserve(fpCount);
    for (int i = 0; i < fpCount; ++i) {
        SkString* inOut = this->pipeline().isColorFragmentProcessor(i) ? color : coverage;
        SkString output;
        const GrFragmentProcessor& fp = this->pipeline().getFragmentProcessor(i);
        fFPImpls.push_back(fp.makeProgramImpl());
        output = this->emitFragProc(fp,
                                    *fFPImpls.back(),
                                    transformedCoordVarsIdx,
                                    *inOut,
                                    output);
        for (const auto& subFP : GrFragmentProcessor::FPRange(fp)) {
            transformedCoordVarsIdx += subFP.numVaryingCoordsUsed();
        }
        *inOut = std::move(output);
    }
}

SkString GrGLSLProgramBuilder::emitFragProc(const GrFragmentProcessor& fp,
                                            GrGLSLFragmentProcessor& glslFP,
                                            int transformedCoordVarsIdx,
                                            const SkString& input,
                                            SkString output) {
    SkASSERT(input.size());
    // Program builders have a bit of state we need to clear with each effect
    AutoStageAdvance adv(this);
    this->nameExpression(&output, "output");
    fFS.codeAppendf("half4 %s;", output.c_str());

    int samplerIdx = 0;
    for (auto [subFP, subGLSLFP] : GrGLSLFragmentProcessor::ParallelRange(fp, glslFP)) {
        if (auto* te = subFP.asTextureEffect()) {
            SkString name;
            name.printf("TextureSampler_%d", samplerIdx++);

            GrSamplerState samplerState = te->samplerState();
            const GrBackendFormat& format = te->view().proxy()->backendFormat();
            GrSwizzle swizzle = te->view().swizzle();
            SamplerHandle handle = this->emitSampler(format, samplerState, swizzle, name.c_str());
            static_cast<GrTextureEffect::Impl&>(subGLSLFP).setSamplerHandle(handle);
        }
    }
    const GrShaderVar* coordVars = fTransformedCoordVars.begin() + transformedCoordVarsIdx;
    GrGLSLFragmentProcessor::TransformedCoordVars coords(&fp, coordVars);
    GrGLSLFragmentProcessor::EmitArgs args(&fFS,
                                           this->uniformHandler(),
                                           this->shaderCaps(),
                                           fp,
                                           "_input",
                                           "_coords",
                                           coords);
    auto name = fFS.writeProcessorFunction(&glslFP, args);
    fFS.codeAppendf("%s = %s(%s);", output.c_str(), name.c_str(), input.c_str());

    // We have to check that effects and the code they emit are consistent, ie if an effect asks
    // for dst color, then the emit code needs to follow suit
    SkDEBUGCODE(verify(fp);)

    return output;
}

void GrGLSLProgramBuilder::emitAndInstallXferProc(const SkString& colorIn,
                                                  const SkString& coverageIn) {
    // Program builders have a bit of state we need to clear with each effect
    AutoStageAdvance adv(this);

    SkASSERT(!fXferProcessor);
    const GrXferProcessor& xp = this->pipeline().getXferProcessor();
    fXferProcessor.reset(xp.createGLSLInstance());

    // Enable dual source secondary output if we have one
    if (xp.hasSecondaryOutput()) {
        fFS.enableSecondaryOutput();
    }

    if (this->shaderCaps()->mustDeclareFragmentShaderOutput()) {
        fFS.enableCustomOutput();
    }

    SkString openBrace;
    openBrace.printf("{ // Xfer Processor: %s\n", xp.name());
    fFS.codeAppend(openBrace.c_str());

    SamplerHandle dstTextureSamplerHandle;
    GrSurfaceOrigin dstTextureOrigin = kTopLeft_GrSurfaceOrigin;

    const GrSurfaceProxyView& dstView = this->pipeline().dstProxyView();
    if (this->pipeline().usesDstTexture()) {
        GrTextureProxy* dstTextureProxy = dstView.asTextureProxy();
        SkASSERT(dstTextureProxy);
        const GrSwizzle& swizzle = dstView.swizzle();
        dstTextureSamplerHandle = this->emitSampler(dstTextureProxy->backendFormat(),
                                                    GrSamplerState(), swizzle, "DstTextureSampler");
        dstTextureOrigin = dstView.origin();
        SkASSERT(dstTextureProxy->textureType() != GrTextureType::kExternal);
    } else if (this->pipeline().usesInputAttachment()) {
        const GrSwizzle& swizzle = dstView.swizzle();
        dstTextureSamplerHandle = this->emitInputSampler(swizzle, "DstTextureInput");
    }

    SkString finalInColor = colorIn.size() ? colorIn : SkString("float4(1)");

    GrGLSLXferProcessor::EmitArgs args(&fFS,
                                       this->uniformHandler(),
                                       this->shaderCaps(),
                                       xp,
                                       finalInColor.c_str(),
                                       coverageIn.size() ? coverageIn.c_str() : "float4(1)",
                                       fFS.getPrimaryColorOutputName(),
                                       fFS.getSecondaryColorOutputName(),
                                       this->pipeline().dstSampleType(),
                                       dstTextureSamplerHandle,
                                       dstTextureOrigin,
                                       this->pipeline().writeSwizzle());
    fXferProcessor->emitCode(args);

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    SkDEBUGCODE(verify(xp);)
    fFS.codeAppend("}");
}

GrGLSLProgramBuilder::SamplerHandle GrGLSLProgramBuilder::emitSampler(
        const GrBackendFormat& backendFormat, GrSamplerState state, const GrSwizzle& swizzle,
        const char* name) {
    ++fNumFragmentSamplers;
    return this->uniformHandler()->addSampler(backendFormat, state, swizzle, name,
                                              this->shaderCaps());
}

GrGLSLProgramBuilder::SamplerHandle GrGLSLProgramBuilder::emitInputSampler(const GrSwizzle& swizzle,
                                                                           const char* name) {
    return this->uniformHandler()->addInputSampler(swizzle, name);
}

bool GrGLSLProgramBuilder::checkSamplerCounts() {
    const GrShaderCaps& shaderCaps = *this->shaderCaps();
    if (fNumFragmentSamplers > shaderCaps.maxFragmentSamplers()) {
        GrCapsDebugf(this->caps(), "Program would use too many fragment samplers\n");
        return false;
    }
    return true;
}

#ifdef SK_DEBUG
void GrGLSLProgramBuilder::verify(const GrPrimitiveProcessor& gp) {
    SkASSERT(!fFS.fHasReadDstColorThisStage_DebugOnly);
    SkASSERT(fFS.fUsedProcessorFeaturesThisStage_DebugOnly == gp.requestedFeatures());
}

void GrGLSLProgramBuilder::verify(const GrFragmentProcessor& fp) {
    SkASSERT(!fFS.fHasReadDstColorThisStage_DebugOnly);
    SkASSERT(fFS.fUsedProcessorFeaturesThisStage_DebugOnly == fp.requestedFeatures());
}

void GrGLSLProgramBuilder::verify(const GrXferProcessor& xp) {
    SkASSERT(xp.willReadDstColor() == fFS.fHasReadDstColorThisStage_DebugOnly);
    SkASSERT(fFS.fUsedProcessorFeaturesThisStage_DebugOnly == xp.requestedFeatures());
}
#endif

SkString GrGLSLProgramBuilder::nameVariable(char prefix, const char* name, bool mangle) {
    SkString out;
    if ('\0' == prefix) {
        out = name;
    } else {
        out.printf("%c%s", prefix, name);
    }
    if (mangle) {
        // Names containing "__" are reserved; add "x" if needed to avoid consecutive underscores.
        const char *underscoreSplitter = out.endsWith('_') ? "x" : "";

        out.appendf("%s_Stage%d%s", underscoreSplitter, fStageIndex, fFS.getMangleString().c_str());
    }
    return out;
}

void GrGLSLProgramBuilder::nameExpression(SkString* output, const char* baseName) {
    // Name a variable to hold stage result. If we already have a valid output name, use that as-is;
    // otherwise, create a new mangled one.
    if (output->isEmpty()) {
        *output = this->nameVariable(/*prefix=*/'\0', baseName);
    }
}

void GrGLSLProgramBuilder::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
    this->uniformHandler()->appendUniformDecls(visibility, out);
}

void GrGLSLProgramBuilder::addRTWidthUniform(const char* name) {
    SkASSERT(!fUniformHandles.fRTWidthUni.isValid());
    GrGLSLUniformHandler* uniformHandler = this->uniformHandler();
    fUniformHandles.fRTWidthUni =
            uniformHandler->internalAddUniformArray(nullptr, kFragment_GrShaderFlag, kHalf_GrSLType,
                                                    name, false, 0, nullptr);
}

void GrGLSLProgramBuilder::addRTHeightUniform(const char* name) {
    SkASSERT(!fUniformHandles.fRTHeightUni.isValid());
    GrGLSLUniformHandler* uniformHandler = this->uniformHandler();
    fUniformHandles.fRTHeightUni =
            uniformHandler->internalAddUniformArray(nullptr, kFragment_GrShaderFlag, kHalf_GrSLType,
                                                    name, false, 0, nullptr);
}

void GrGLSLProgramBuilder::finalizeShaders() {
    this->varyingHandler()->finalize();
    fVS.finalize(kVertex_GrShaderFlag);
    if (this->primitiveProcessor().willUseGeoShader()) {
        SkASSERT(this->shaderCaps()->geometryShaderSupport());
        fGS.finalize(kGeometry_GrShaderFlag);
    }
    fFS.finalize(kFragment_GrShaderFlag);
}
