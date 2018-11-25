/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShadowGeoProc.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

class GrGLSLRRectShadowGeoProc : public GrGLSLGeometryProcessor {
public:
    GrGLSLRRectShadowGeoProc() {}

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const GrRRectShadowGeoProc& rsgp = args.fGP.cast<GrRRectShadowGeoProc>();
        GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
        GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;

        // emit attributes
        varyingHandler->emitAttributes(rsgp);
        fragBuilder->codeAppend("half4 shadowParams;");
        varyingHandler->addPassThroughAttribute(rsgp.inShadowParams(), "shadowParams");

        // setup pass through color
        varyingHandler->addPassThroughAttribute(rsgp.inColor(), args.fOutputColor);

        // Setup position
        this->writeOutputPosition(vertBuilder, gpArgs, rsgp.inPosition()->fName);

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             rsgp.inPosition()->asShaderVar(),
                             args.fFPCoordTransformHandler);

        fragBuilder->codeAppend("half d = length(shadowParams.xy);");
        fragBuilder->codeAppend("half distance = shadowParams.z * (1.0 - d);");

        fragBuilder->codeAppend("half factor = 1.0 - clamp(distance, 0.0, shadowParams.w);");
        fragBuilder->codeAppend("factor = exp(-factor * factor * 4.0) - 0.018;");
        fragBuilder->codeAppendf("%s = half4(factor);",
                                 args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                 FPCoordTransformIter&& transformIter) override {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

private:
    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrRRectShadowGeoProc::GrRRectShadowGeoProc()
: INHERITED(kGrRRectShadowGeoProc_ClassID) {
    fInPosition = &this->addVertexAttrib("inPosition", kFloat2_GrVertexAttribType);
    fInColor = &this->addVertexAttrib("inColor", kUByte4_norm_GrVertexAttribType);
    fInShadowParams = &this->addVertexAttrib("inShadowParams", kHalf4_GrVertexAttribType);
}

GrGLSLPrimitiveProcessor* GrRRectShadowGeoProc::createGLSLInstance(const GrShaderCaps&) const {
    return new GrGLSLRRectShadowGeoProc();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrRRectShadowGeoProc);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> GrRRectShadowGeoProc::TestCreate(GrProcessorTestData* d) {
    return GrRRectShadowGeoProc::Make();
}
#endif
