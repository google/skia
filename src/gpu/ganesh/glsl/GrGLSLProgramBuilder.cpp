/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"

#include <memory>

#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/sksl/SkSLCompiler.h"

using namespace skia_private;

const int GrGLSLProgramBuilder::kVarsPerBlock = 8;

GrGLSLProgramBuilder::GrGLSLProgramBuilder(const GrProgramDesc& desc,
                                           const GrProgramInfo& programInfo)
        : fVS(this)
        , fFS(this)
        , fDesc(desc)
        , fProgramInfo(programInfo)
        , fNumFragmentSamplers(0) {}

GrGLSLProgramBuilder::~GrGLSLProgramBuilder() = default;

void GrGLSLProgramBuilder::addFeature(GrShaderFlags shaders,
                                      uint32_t featureBit,
                                      const char* extensionName) {
    if (shaders & kVertex_GrShaderFlag) {
        fVS.addFeature(featureBit, extensionName);
    }
    if (shaders & kFragment_GrShaderFlag) {
        fFS.addFeature(featureBit, extensionName);
    }
}

bool GrGLSLProgramBuilder::emitAndInstallProcs() {
    // First we loop over all of the installed processors and collect coord transforms.  These will
    // be sent to the ProgramImpl in its emitCode function
    SkString inputColor;
    SkString inputCoverage;
    if (!this->emitAndInstallPrimProc(&inputColor, &inputCoverage)) {
        return false;
    }
    if (!this->emitAndInstallDstTexture()) {
        return false;
    }
    if (!this->emitAndInstallFragProcs(&inputColor, &inputCoverage)) {
        return false;
    }
    if (!this->emitAndInstallXferProc(inputColor, inputCoverage)) {
        return false;
    }
    fGPImpl->emitTransformCode(&fVS, this->uniformHandler());

    return this->checkSamplerCounts();
}

bool GrGLSLProgramBuilder::emitAndInstallPrimProc(SkString* outputColor, SkString* outputCoverage) {
    const GrGeometryProcessor& geomProc = this->geometryProcessor();

    // Program builders have a bit of state we need to clear with each effect
    this->advanceStage();
    this->nameExpression(outputColor, "outputColor");
    this->nameExpression(outputCoverage, "outputCoverage");

    SkASSERT(!fUniformHandles.fRTAdjustmentUni.isValid());
    fUniformHandles.fRTAdjustmentUni = this->uniformHandler()->addUniform(
            nullptr, kVertex_GrShaderFlag, SkSLType::kFloat4, SkSL::Compiler::RTADJUST_NAME);

    fFS.codeAppendf("// Stage %d, %s\n", fStageIndex, geomProc.name());
    fVS.codeAppendf("// Primitive Processor %s\n", geomProc.name());

    SkASSERT(!fGPImpl);
    fGPImpl = geomProc.makeProgramImpl(*this->shaderCaps());

    AutoSTArray<4, SamplerHandle> texSamplers(geomProc.numTextureSamplers());
    for (int i = 0; i < geomProc.numTextureSamplers(); ++i) {
        SkString name;
        name.printf("TextureSampler_%d", i);
        const auto& sampler = geomProc.textureSampler(i);
        texSamplers[i] = this->emitSampler(geomProc.textureSampler(i).backendFormat(),
                                           sampler.samplerState(),
                                           sampler.swizzle(),
                                           name.c_str());
        if (!texSamplers[i].isValid()) {
            return false;
        }
    }

    GrGeometryProcessor::ProgramImpl::EmitArgs args(&fVS,
                                                    &fFS,
                                                    this->varyingHandler(),
                                                    this->uniformHandler(),
                                                    this->shaderCaps(),
                                                    geomProc,
                                                    outputColor->c_str(),
                                                    outputCoverage->c_str(),
                                                    texSamplers.get());
    std::tie(fFPCoordsMap, fLocalCoordsVar) = fGPImpl->emitCode(args, this->pipeline());

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    SkDEBUGCODE(verify(geomProc);)

    return true;
}

bool GrGLSLProgramBuilder::emitAndInstallFragProcs(SkString* color, SkString* coverage) {
    int fpCount = this->pipeline().numFragmentProcessors();
    SkASSERT(fFPImpls.empty());
    fFPImpls.reserve(fpCount);
    for (int i = 0; i < fpCount; ++i) {
        SkString* inOut = this->pipeline().isColorFragmentProcessor(i) ? color : coverage;
        SkString output;
        const GrFragmentProcessor& fp = this->pipeline().getFragmentProcessor(i);
        fFPImpls.push_back(fp.makeProgramImpl());
        output = this->emitRootFragProc(fp, *fFPImpls.back(), *inOut, output);
        if (output.isEmpty()) {
            return false;
        }
        *inOut = std::move(output);
    }
    return true;
}

bool GrGLSLProgramBuilder::emitTextureSamplersForFPs(const GrFragmentProcessor& fp,
                                                     GrFragmentProcessor::ProgramImpl& impl,
                                                     int* samplerIndex) {
    bool ok = true;
    fp.visitWithImpls([&](const GrFragmentProcessor& fp, GrFragmentProcessor::ProgramImpl& impl) {
        if (const GrTextureEffect* te = fp.asTextureEffect()) {
            SkString name = SkStringPrintf("TextureSampler_%d", *samplerIndex);
            *samplerIndex += 1;

            GrSamplerState samplerState = te->samplerState();
            const GrBackendFormat& format = te->view().proxy()->backendFormat();
            skgpu::Swizzle swizzle = te->view().swizzle();
            SamplerHandle handle = this->emitSampler(format, samplerState, swizzle, name.c_str());
            if (!handle.isValid()) {
                ok = false;
                return;
            }
            static_cast<GrTextureEffect::Impl&>(impl).setSamplerHandle(handle);
        }
    }, impl);

    return ok;
}

std::string GrGLSLProgramBuilder::invokeFP(const GrFragmentProcessor& fp,
                                           const GrFragmentProcessor::ProgramImpl& impl,
                                           const char* inputColor,
                                           const char* destColor,
                                           const char* coords) const {
    if (fp.isBlendFunction()) {
        if (this->fragmentProcessorHasCoordsParam(&fp)) {
            return SkSL::String::printf("%s(%s, %s, %s)", impl.functionName(), inputColor,
                                                          destColor, coords);
        } else {
            return SkSL::String::printf("%s(%s, %s)", impl.functionName(), inputColor, destColor);
        }
    }

    if (this->fragmentProcessorHasCoordsParam(&fp)) {
        return SkSL::String::printf("%s(%s, %s)", impl.functionName(), inputColor, coords);
    } else {
        return SkSL::String::printf("%s(%s)", impl.functionName(), inputColor);
    }
}

SkString GrGLSLProgramBuilder::emitRootFragProc(const GrFragmentProcessor& fp,
                                                GrFragmentProcessor::ProgramImpl& impl,
                                                const SkString& input,
                                                SkString output) {
    SkASSERT(input.size());

    // Program builders have a bit of state we need to clear with each effect
    this->advanceStage();
    this->nameExpression(&output, "output");
    fFS.codeAppendf("half4 %s;", output.c_str());
    int samplerIndex = 0;
    if (!this->emitTextureSamplersForFPs(fp, impl, &samplerIndex)) {
        return {};
    }

    this->writeFPFunction(fp, impl);

    fFS.codeAppendf(
            "%s = %s;",
            output.c_str(),
            this->invokeFP(fp, impl, input.c_str(), "half4(1)", fLocalCoordsVar.c_str()).c_str());

    // We have to check that effects and the code they emit are consistent, ie if an effect asks
    // for dst color, then the emit code needs to follow suit
    SkDEBUGCODE(verify(fp);)

    return output;
}

void GrGLSLProgramBuilder::writeChildFPFunctions(const GrFragmentProcessor& fp,
                                                 GrFragmentProcessor::ProgramImpl& impl) {
    fSubstageIndices.push_back(0);
    for (int i = 0; i < impl.numChildProcessors(); ++i) {
        GrFragmentProcessor::ProgramImpl* childImpl = impl.childProcessor(i);
        if (!childImpl) {
            continue;
        }

        const GrFragmentProcessor* childFP = fp.childProcessor(i);
        SkASSERT(childFP);

        this->writeFPFunction(*childFP, *childImpl);
        ++fSubstageIndices.back();
    }
    fSubstageIndices.pop_back();
}

void GrGLSLProgramBuilder::writeFPFunction(const GrFragmentProcessor& fp,
                                           GrFragmentProcessor::ProgramImpl& impl) {
    constexpr const char*       kDstColor    = "_dst";
              const char* const inputColor   = fp.isBlendFunction() ? "_src" : "_input";
              const char*       sampleCoords = "_coords";
    fFS.nextStage();
    // Conceptually, an FP is always sampled at a particular coordinate. However, if it is only
    // sampled by a chain of uniform matrix expressions (or legacy coord transforms), the value that
    // would have been passed to _coords is lifted to the vertex shader and
    // varying. In that case it uses that variable and we do not pass a second argument for _coords.
    GrShaderVar params[3];
    int numParams = 0;

    params[numParams++] = GrShaderVar(inputColor, SkSLType::kHalf4);

    if (fp.isBlendFunction()) {
        // Blend functions take a dest color as input.
        params[numParams++] = GrShaderVar(kDstColor, SkSLType::kHalf4);
    }

    if (this->fragmentProcessorHasCoordsParam(&fp)) {
        params[numParams++] = GrShaderVar(sampleCoords, SkSLType::kFloat2);
    } else {
        // Either doesn't use coords at all or sampled through a chain of passthrough/matrix
        // samples usages. In the latter case the coords are emitted in the vertex shader as a
        // varying, so this only has to access it. Add a float2 _coords variable that maps to the
        // associated varying and replaces the absent 2nd argument to the fp's function.
        GrShaderVar varying = fFPCoordsMap[&fp].coordsVarying;

        switch (varying.getType()) {
            case SkSLType::kVoid:
                SkASSERT(!fp.usesSampleCoordsDirectly());
                break;
            case SkSLType::kFloat2:
                // Just point the local coords to the varying
                sampleCoords = varying.getName().c_str();
                break;
            case SkSLType::kFloat3:
                // Must perform the perspective divide in the frag shader based on the
                // varying, and since we won't actually have a function parameter for local
                // coords, add it as a local variable.
                fFS.codeAppendf("float2 %s = %s.xy / %s.z;\n",
                                sampleCoords,
                                varying.getName().c_str(),
                                varying.getName().c_str());
                break;
            default:
                SkDEBUGFAILF("Unexpected varying type for coord: %s %d\n",
                             varying.getName().c_str(),
                             (int)varying.getType());
                break;
        }
    }

    SkASSERT(numParams <= (int)std::size(params));

    // First, emit every child's function. This needs to happen (even for children that aren't
    // sampled), so that all of the expected uniforms are registered.
    this->writeChildFPFunctions(fp, impl);
    GrFragmentProcessor::ProgramImpl::EmitArgs args(&fFS,
                                                    this->uniformHandler(),
                                                    this->shaderCaps(),
                                                    fp,
                                                    inputColor,
                                                    kDstColor,
                                                    sampleCoords);

    impl.emitCode(args);
    impl.setFunctionName(fFS.getMangledFunctionName(args.fFp.name()));

    fFS.emitFunction(SkSLType::kHalf4,
                     impl.functionName(),
                     SkSpan(params, numParams),
                     fFS.code().c_str());
    fFS.deleteStage();
}

bool GrGLSLProgramBuilder::emitAndInstallDstTexture() {
    fDstTextureOrigin = kTopLeft_GrSurfaceOrigin;

    const GrSurfaceProxyView& dstView = this->pipeline().dstProxyView();
    if (this->pipeline().usesDstTexture()) {
        // Set up a sampler handle for the destination texture.
        GrTextureProxy* dstTextureProxy = dstView.asTextureProxy();
        SkASSERT(dstTextureProxy);
        const skgpu::Swizzle& swizzle = dstView.swizzle();
        fDstTextureSamplerHandle = this->emitSampler(dstTextureProxy->backendFormat(),
                                                    GrSamplerState(), swizzle, "DstTextureSampler");
        if (!fDstTextureSamplerHandle.isValid()) {
            return false;
        }
        fDstTextureOrigin = dstView.origin();
        SkASSERT(dstTextureProxy->textureType() != GrTextureType::kExternal);

        // Declare a _dstColor global variable which samples from the dest-texture sampler at the
        // top of the fragment shader.
        const char* dstTextureCoordsName;
        fUniformHandles.fDstTextureCoordsUni = this->uniformHandler()->addUniform(
                /*owner=*/nullptr,
                kFragment_GrShaderFlag,
                SkSLType::kHalf4,
                "DstTextureCoords",
                &dstTextureCoordsName);
        fFS.codeAppend("// Read color from copy of the destination\n");
        if (dstTextureProxy->textureType() == GrTextureType::k2D) {
            fFS.codeAppendf("float2 _dstTexCoord = (sk_FragCoord.xy - %s.xy) * %s.zw;\n",
                    dstTextureCoordsName, dstTextureCoordsName);
            if (fDstTextureOrigin == kBottomLeft_GrSurfaceOrigin) {
                fFS.codeAppend("_dstTexCoord.y = 1.0 - _dstTexCoord.y;\n");
            }
        } else {
            SkASSERT(dstTextureProxy->textureType() == GrTextureType::kRectangle);
            fFS.codeAppendf("float2 _dstTexCoord = sk_FragCoord.xy - %s.xy;\n",
                    dstTextureCoordsName);
            if (fDstTextureOrigin == kBottomLeft_GrSurfaceOrigin) {
                // When the texture type is kRectangle, instead of a scale stored in the zw of the
                // uniform, we store the height in z so we can flip the coord here.
                fFS.codeAppendf("_dstTexCoord.y = %s.z - _dstTexCoord.y;\n", dstTextureCoordsName);
            }
        }
        const char* dstColor = fFS.dstColor();
        SkString dstColorDecl = SkStringPrintf("half4 %s;", dstColor);
        fFS.definitionAppend(dstColorDecl.c_str());
        fFS.codeAppendf("%s = ", dstColor);
        fFS.appendTextureLookup(fDstTextureSamplerHandle, "_dstTexCoord");
        fFS.codeAppend(";\n");
    } else if (this->pipeline().usesDstInputAttachment()) {
        // Set up an input attachment for the destination texture.
        const skgpu::Swizzle& swizzle = dstView.swizzle();
        fDstTextureSamplerHandle = this->emitInputSampler(swizzle, "DstTextureInput");
        if (!fDstTextureSamplerHandle.isValid()) {
            return false;
        }

        // Populate the _dstColor variable by loading from the input attachment at the top of the
        // fragment shader.
        fFS.codeAppend("// Read color from input attachment\n");
        const char* dstColor = fFS.dstColor();
        SkString dstColorDecl = SkStringPrintf("half4 %s;", dstColor);
        fFS.definitionAppend(dstColorDecl.c_str());
        fFS.codeAppendf("%s = ", dstColor);
        fFS.appendInputLoad(fDstTextureSamplerHandle);
        fFS.codeAppend(";\n");
    }

    return true;
}

bool GrGLSLProgramBuilder::emitAndInstallXferProc(const SkString& colorIn,
                                                  const SkString& coverageIn) {
    // Program builders have a bit of state we need to clear with each effect
    this->advanceStage();

    SkASSERT(!fXPImpl);
    const GrXferProcessor& xp = this->pipeline().getXferProcessor();
    fXPImpl = xp.makeProgramImpl();

    // Enable dual source secondary output if we have one
    if (xp.hasSecondaryOutput()) {
        fFS.enableSecondaryOutput();
    }

    SkString openBrace;
    openBrace.printf("{ // Xfer Processor: %s\n", xp.name());
    fFS.codeAppend(openBrace.c_str());

    SkString finalInColor = colorIn.size() ? colorIn : SkString("float4(1)");

    GrXferProcessor::ProgramImpl::EmitArgs args(
            &fFS,
            this->uniformHandler(),
            this->shaderCaps(),
            xp,
            finalInColor.c_str(),
            coverageIn.size() ? coverageIn.c_str() : "float4(1)",
            fFS.getPrimaryColorOutputName(),
            fFS.getSecondaryColorOutputName(),
            fDstTextureSamplerHandle,
            fDstTextureOrigin,
            this->pipeline().writeSwizzle());
    fXPImpl->emitCode(args);

    // We have to check that effects and the code they emit are consistent, ie if an effect
    // asks for dst color, then the emit code needs to follow suit
    SkDEBUGCODE(verify(xp);)
    fFS.codeAppend("}");
    return true;
}

GrGLSLProgramBuilder::SamplerHandle GrGLSLProgramBuilder::emitSampler(
        const GrBackendFormat& backendFormat, GrSamplerState state, const skgpu::Swizzle& swizzle,
        const char* name) {
    ++fNumFragmentSamplers;
    return this->uniformHandler()->addSampler(backendFormat, state, swizzle, name,
                                              this->shaderCaps());
}

GrGLSLProgramBuilder::SamplerHandle GrGLSLProgramBuilder::emitInputSampler(
        const skgpu::Swizzle& swizzle, const char* name) {
    return this->uniformHandler()->addInputSampler(swizzle, name);
}

bool GrGLSLProgramBuilder::checkSamplerCounts() {
    const GrShaderCaps& shaderCaps = *this->shaderCaps();
    if (fNumFragmentSamplers > shaderCaps.fMaxFragmentSamplers) {
        GrCapsDebugf(this->caps(), "Program would use too many fragment samplers\n");
        return false;
    }
    return true;
}

#ifdef SK_DEBUG
void GrGLSLProgramBuilder::verify(const GrGeometryProcessor& geomProc) {
    SkASSERT(!fFS.fHasReadDstColorThisStage_DebugOnly);
}

void GrGLSLProgramBuilder::verify(const GrFragmentProcessor& fp) {
    SkASSERT(fp.willReadDstColor() == fFS.fHasReadDstColorThisStage_DebugOnly);
}

void GrGLSLProgramBuilder::verify(const GrXferProcessor& xp) {
    SkASSERT(xp.willReadDstColor() == fFS.fHasReadDstColorThisStage_DebugOnly);
}
#endif

SkString GrGLSLProgramBuilder::getMangleSuffix() const {
    SkASSERT(fStageIndex >= 0);
    SkString suffix;
    suffix.printf("_S%d", fStageIndex);
    for (auto c : fSubstageIndices) {
        suffix.appendf("_c%d", c);
    }
    return suffix;
}

SkString GrGLSLProgramBuilder::nameVariable(char prefix, const char* name, bool mangle) {
    SkString out;
    if ('\0' == prefix) {
        out = name;
    } else {
        out.printf("%c%s", prefix, name);
    }
    if (mangle) {
        SkString suffix = this->getMangleSuffix();
        // Names containing "__" are reserved; add "x" if needed to avoid consecutive underscores.
        const char *underscoreSplitter = out.endsWith('_') ? "x" : "";
        out.appendf("%s%s", underscoreSplitter, suffix.c_str());
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

void GrGLSLProgramBuilder::addRTFlipUniform(const char* name) {
    SkASSERT(!fUniformHandles.fRTFlipUni.isValid());
    GrGLSLUniformHandler* uniformHandler = this->uniformHandler();
    fUniformHandles.fRTFlipUni =
            uniformHandler->internalAddUniformArray(nullptr,
                                                    kFragment_GrShaderFlag,
                                                    SkSLType::kFloat2,
                                                    name,
                                                    false,
                                                    0,
                                                    nullptr);
}

bool GrGLSLProgramBuilder::fragmentProcessorHasCoordsParam(const GrFragmentProcessor* fp) const {
    auto iter = fFPCoordsMap.find(fp);
    return (iter != fFPCoordsMap.end()) ? iter->second.hasCoordsParam
                                        : fp->usesSampleCoords();
}

void GrGLSLProgramBuilder::finalizeShaders() {
    this->varyingHandler()->finalize();
    fVS.finalize(kVertex_GrShaderFlag);
    fFS.finalize(kFragment_GrShaderFlag);
}
