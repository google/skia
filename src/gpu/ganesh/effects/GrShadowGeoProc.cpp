/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrShadowGeoProc.h"

#include "include/core/SkSamplingOptions.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"

class GrGLSLProgramDataManager;
class GrGLSLVertexBuilder;
struct GrShaderCaps;

class GrRRectShadowGeoProc::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager&,
                 const GrShaderCaps&,
                 const GrGeometryProcessor&) override {}

private:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const GrRRectShadowGeoProc& rsgp = args.fGeomProc.cast<GrRRectShadowGeoProc>();
        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        // emit attributes
        varyingHandler->emitAttributes(rsgp);
        fragBuilder->codeAppend("half3 shadowParams;");
        varyingHandler->addPassThroughAttribute(rsgp.inShadowParams().asShaderVar(),
                                                "shadowParams");

        // setup pass through color
        fragBuilder->codeAppendf("half4 %s;", args.fOutputColor);
        varyingHandler->addPassThroughAttribute(rsgp.inColor().asShaderVar(), args.fOutputColor);

        // Setup position
        WriteOutputPosition(vertBuilder, gpArgs, rsgp.inPosition().name());
        // No need for local coordinates, this GP does not combine with fragment processors

        fragBuilder->codeAppend("half d = length(shadowParams.xy);");
        fragBuilder->codeAppend("float2 uv = float2(shadowParams.z * (1.0 - d), 0.5);");
        fragBuilder->codeAppend("half factor = ");
        fragBuilder->appendTextureLookup(args.fTexSamplers[0], "uv");
        fragBuilder->codeAppend(".a;");
        fragBuilder->codeAppendf("half4 %s = half4(factor);", args.fOutputCoverage);
    }
};

///////////////////////////////////////////////////////////////////////////////

GrRRectShadowGeoProc::GrRRectShadowGeoProc(const GrSurfaceProxyView& lutView)
        : INHERITED(kGrRRectShadowGeoProc_ClassID) {
    fInPosition = {"inPosition", kFloat2_GrVertexAttribType, SkSLType::kFloat2};
    fInColor = {"inColor", kUByte4_norm_GrVertexAttribType, SkSLType::kHalf4};
    fInShadowParams = {"inShadowParams", kFloat3_GrVertexAttribType, SkSLType::kHalf3};
    this->setVertexAttributesWithImplicitOffsets(&fInPosition, 3);

    SkASSERT(lutView.proxy());
    fLUTTextureSampler.reset(GrSamplerState::Filter::kLinear, lutView.proxy()->backendFormat(),
                             lutView.swizzle());
    this->setTextureSamplerCnt(1);
}

std::unique_ptr<GrGeometryProcessor::ProgramImpl> GrRRectShadowGeoProc::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrRRectShadowGeoProc)

#if defined(GR_TEST_UTILS)
GrGeometryProcessor* GrRRectShadowGeoProc::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomAlphaOnlyView();

    return GrRRectShadowGeoProc::Make(d->allocator(), view);
}
#endif
