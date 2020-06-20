/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrShadowGeoProc.h"

#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

class GrGLSLRRectShadowGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLSLRRectShadowGeoProc() {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const GrRRectShadowGeoProc& rsgp = args.fGP.cast<GrRRectShadowGeoProc>();
        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

        // emit attributes
        varyingHandler->emitAttributes(rsgp);
        fragBuilder->codeAppend("half3 shadowParams;");
        varyingHandler->addPassThroughAttribute(rsgp.inShadowParams(), "shadowParams");

        // setup pass through color
        varyingHandler->addPassThroughAttribute(rsgp.inColor(), args.fOutputColor);

        // Setup position
        this->writeOutputPosition(vertBuilder, gpArgs, rsgp.inPosition().name());

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             rsgp.inPosition().asShaderVar(),
                             args.fFPCoordTransformHandler);

        fragBuilder->codeAppend("half d = length(shadowParams.xy);");
        fragBuilder->codeAppend("float2 uv = float2(shadowParams.z * (1.0 - d), 0.5);");
        fragBuilder->codeAppend("half factor = ");
        fragBuilder->appendTextureLookup(args.fTexSamplers[0], "uv");
        fragBuilder->codeAppend(".a;");
        fragBuilder->codeAppendf("%s = half4(factor);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                 const CoordTransformRange& transformRange) override {
        this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
    }

private:
    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrRRectShadowGeoProc::GrRRectShadowGeoProc(const GrSurfaceProxyView& lutView)
        : INHERITED(kGrRRectShadowGeoProc_ClassID) {
    fInPosition = {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    fInColor = {"inColor", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType};
    fInShadowParams = {"inShadowParams", kFloat3_GrVertexAttribType, kHalf3_GrSLType};
    this->setVertexAttributes(&fInPosition, 3);

    SkASSERT(lutView.proxy());
    fLUTTextureSampler.reset(GrSamplerState::Filter::kBilerp, lutView.proxy()->backendFormat(),
                             lutView.swizzle());
    this->setTextureSamplerCnt(1);
}

GrGLSLPrimitiveProcessor* GrRRectShadowGeoProc::createGLSLInstance(const GrShaderCaps&) const {
    return new GrGLSLRRectShadowGeoProc();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrRRectShadowGeoProc);

#if GR_TEST_UTILS
GrGeometryProcessor* GrRRectShadowGeoProc::TestCreate(GrProcessorTestData* d) {
    auto [view, ct, at] = d->randomAlphaOnlyView();

    return GrRRectShadowGeoProc::Make(d->allocator(), view);
}
#endif
