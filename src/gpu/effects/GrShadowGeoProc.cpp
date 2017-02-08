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
#include "glsl/GrGLSLVertexShaderBuilder.h"

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
        fragBuilder->codeAppend("vec4 shadowParams;");
        varyingHandler->addPassThroughAttribute(rsgp.inShadowParams(), "shadowParams");

        // setup pass through color
        varyingHandler->addPassThroughAttribute(rsgp.inColor(), args.fOutputColor);

        // Setup position
        this->setupPosition(vertBuilder, gpArgs, rsgp.inPosition()->fName);

        // emit transforms
        this->emitTransforms(vertBuilder,
                             varyingHandler,
                             uniformHandler,
                             gpArgs->fPositionVar,
                             rsgp.inPosition()->fName,
                             rsgp.localMatrix(),
                             args.fFPCoordTransformHandler);

        fragBuilder->codeAppend("float d = length(shadowParams.xy);");
        fragBuilder->codeAppend("float distance = shadowParams.z * (1.0 - d);");

        fragBuilder->codeAppend("float radius = shadowParams.w;");
        
        fragBuilder->codeAppend("float factor = 1.0 - clamp(distance/radius, 0.0, 1.0);");
        fragBuilder->codeAppend("factor = exp(-factor * factor * 4.0) - 0.018;");
        fragBuilder->codeAppendf("%s = vec4(factor);",
                                 args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                 FPCoordTransformIter&& transformIter) override {
        this->setTransformDataHelper(proc.cast<GrRRectShadowGeoProc>().localMatrix(),
                                     pdman, &transformIter);
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrRRectShadowGeoProc& rsgp = gp.cast<GrRRectShadowGeoProc>();
        uint16_t key;
        key = rsgp.localMatrix().hasPerspective() ? 0x1 : 0x0;
        b->add32(key);
    }

private:
    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrRRectShadowGeoProc::GrRRectShadowGeoProc(const SkMatrix& localMatrix)
    : fLocalMatrix(localMatrix) {

    this->initClassID<GrRRectShadowGeoProc>();
    fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                         kHigh_GrSLPrecision);
    fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
    fInShadowParams = &this->addVertexAttrib("inShadowParams", kVec4f_GrVertexAttribType);
}

void GrRRectShadowGeoProc::getGLSLProcessorKey(const GrShaderCaps& caps,
                                               GrProcessorKeyBuilder* b) const {
    GrGLSLRRectShadowGeoProc::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrRRectShadowGeoProc::createGLSLInstance(const GrShaderCaps&) const {
    return new GrGLSLRRectShadowGeoProc();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrRRectShadowGeoProc);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> GrRRectShadowGeoProc::TestCreate(GrProcessorTestData* d) {
    return GrRRectShadowGeoProc::Make(GrTest::TestMatrix(d->fRandom));
}
#endif
