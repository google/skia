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
        fragBuilder->codeAppend("vec4 circleEdge;");
        varyingHandler->addPassThroughAttribute(rsgp.inCircleEdge(), "circleEdge");

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

        fragBuilder->codeAppend("float d = length(circleEdge.xy);");
        fragBuilder->codeAppend("float distanceToOuterEdge = circleEdge.z * (1.0 - d);");
        fragBuilder->codeAppend("float edgeAlpha = clamp(distanceToOuterEdge, 0.0, 1.0);");
        if (rsgp.stroked()) {
            fragBuilder->codeAppend("float distanceToInnerEdge = circleEdge.z * (d - circleEdge.w);");
            fragBuilder->codeAppend("float innerAlpha = clamp(distanceToInnerEdge, 0.0, 1.0);");
            fragBuilder->codeAppend("edgeAlpha *= innerAlpha;");
        }

        if (args.fDistanceVectorName) {
            const char* innerEdgeDistance = rsgp.stroked() ? "distanceToInnerEdge" : "0.0";
            fragBuilder->codeAppend("if (d == 0.0) {"); // if on the center of the circle
            fragBuilder->codeAppendf("    %s = vec4(1.0, 0.0, distanceToOuterEdge, "
                                     "%s);", // no normalize
                                     args.fDistanceVectorName, innerEdgeDistance);
            fragBuilder->codeAppend("} else {");
            fragBuilder->codeAppendf("    %s = vec4(normalize(circleEdge.xy), distanceToOuterEdge, %s);",
                                     args.fDistanceVectorName, innerEdgeDistance);
            fragBuilder->codeAppend("}");
        }
        fragBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                 FPCoordTransformIter&& transformIter) override {
        this->setTransformDataHelper(proc.cast<GrRRectShadowGeoProc>().localMatrix(),
                                     pdman, &transformIter);
    }

    static inline void GenKey(const GrGeometryProcessor& gp,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrRRectShadowGeoProc& rsgp = gp.cast<GrRRectShadowGeoProc>();
        uint16_t key;
        key = rsgp.stroked() ? 0x01 : 0x0;
        key |= rsgp.localMatrix().hasPerspective() ? 0x02 : 0x0;
        b->add32(key);
    }

private:
    typedef GrGLSLGeometryProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrRRectShadowGeoProc::GrRRectShadowGeoProc(bool stroke, const SkMatrix& localMatrix)
    : fLocalMatrix(localMatrix)
    , fStroke(stroke) {

    this->initClassID<GrRRectShadowGeoProc>();
    fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                         kHigh_GrSLPrecision);
    fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
    fInCircleEdge = &this->addVertexAttrib("inCircleEdge", kVec4f_GrVertexAttribType);
//    fInShadowParams = &this->addVertexAttrib("inShadowParams", kVec2f_GrVertexAttribType);
}

void GrRRectShadowGeoProc::getGLSLProcessorKey(const GrGLSLCaps& caps,
                                                       GrProcessorKeyBuilder* b) const {
    GrGLSLRRectShadowGeoProc::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrRRectShadowGeoProc::createGLSLInstance(const GrGLSLCaps&) const {
    return new GrGLSLRRectShadowGeoProc();
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrRRectShadowGeoProc);

sk_sp<GrGeometryProcessor> GrRRectShadowGeoProc::TestCreate(GrProcessorTestData* d) {
    return GrRRectShadowGeoProc::Make(d->fRandom->nextBool(),
                                      GrTest::TestMatrix(d->fRandom));
}
