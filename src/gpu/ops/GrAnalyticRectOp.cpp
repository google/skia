/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAnalyticRectOp.h"

#include "GrDrawOpTest.h"
#include "GrGeometryProcessor.h"
#include "GrOpFlushState.h"
#include "GrProcessor.h"
#include "GrResourceProvider.h"
#include "SkRRect.h"
#include "SkStrokeRec.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLUtil.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
#include "ops/GrMeshDrawOp.h"

namespace {

struct RectVertex {
    SkPoint fPos;
    GrColor fColor;
    SkPoint fCenter;
    SkVector fDownDir;
    SkScalar fHalfWidth;
    SkScalar fHalfHeight;
};
}

///////////////////////////////////////////////////////////////////////////////

/**
 * The output of this effect is the input color and coverage for an arbitrarily oriented rect. The
 * rect is specified as:
 *      Center of the rect
 *      Unit vector point down the height of the rect
 *      Half width + 0.5
 *      Half height + 0.5
 * The center and vector are stored in a vec4 varying ("RectEdge") with the
 * center in the xy components and the vector in the zw components.
 * The munged width and height are stored in a vec2 varying ("WidthHeight")
 * with the width in x and the height in y.
 */
class RectGeometryProcessor : public GrGeometryProcessor {
public:
    RectGeometryProcessor(const SkMatrix& localMatrix) : fLocalMatrix(localMatrix) {
        this->initClassID<RectGeometryProcessor>();
        fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                             kHigh_GrSLPrecision);
        fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
        fInRectEdge = &this->addVertexAttrib("inRectEdge", kVec4f_GrVertexAttribType);
        fInWidthHeight = &this->addVertexAttrib("inWidthHeight", kVec2f_GrVertexAttribType);
    }

    bool implementsDistanceVector() const override { return true; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inRectEdge() const { return fInRectEdge; }
    const Attribute* inWidthHeight() const { return fInWidthHeight; }

    const SkMatrix& localMatrix() const { return fLocalMatrix; }

    ~RectGeometryProcessor() override {}

    const char* name() const override { return "RectEdge"; }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor() {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const RectGeometryProcessor& rgp = args.fGP.cast<RectGeometryProcessor>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(rgp);

            // setup the varying for the position
            GrGLSLVertToFrag positionVary(kVec2f_GrSLType);
            varyingHandler->addVarying("Position", &positionVary);
            vertBuilder->codeAppendf("%s = %s;", positionVary.vsOut(), rgp.inPosition()->fName);

            // setup the varying for the center point and the unit vector that points down the
            // height of the rect
            GrGLSLVertToFrag rectEdgeVary(kVec4f_GrSLType);
            varyingHandler->addVarying("RectEdge", &rectEdgeVary);
            vertBuilder->codeAppendf("%s = %s;", rectEdgeVary.vsOut(), rgp.inRectEdge()->fName);

            // setup the varying for the width/2+.5 and height/2+.5
            GrGLSLVertToFrag widthHeightVary(kVec2f_GrSLType);
            varyingHandler->addVarying("WidthHeight", &widthHeightVary);
            vertBuilder->codeAppendf("%s = %s;", widthHeightVary.vsOut(),
                                     rgp.inWidthHeight()->fName);

            GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;

            // setup pass through color
            varyingHandler->addPassThroughAttribute(rgp.inColor(), args.fOutputColor);

            // Setup position
            this->setupPosition(vertBuilder, gpArgs, rgp.inPosition()->fName);

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 gpArgs->fPositionVar,
                                 rgp.inPosition()->fName,
                                 rgp.localMatrix(),
                                 args.fFPCoordTransformHandler);

            // TODO: compute all these offsets, spans, and scales in the VS
            fragBuilder->codeAppendf("float insetW = min(1.0, %s.x) - 0.5;",
                                     widthHeightVary.fsIn());
            fragBuilder->codeAppendf("float insetH = min(1.0, %s.y) - 0.5;",
                                     widthHeightVary.fsIn());
            fragBuilder->codeAppend("float outset = 0.5;");
            // For rects > 1 pixel wide and tall the span's are noops (i.e., 1.0). For rects
            // < 1 pixel wide or tall they serve to normalize the < 1 ramp to a 0 .. 1 range.
            fragBuilder->codeAppend("float spanW = insetW + outset;");
            fragBuilder->codeAppend("float spanH = insetH + outset;");
            // For rects < 1 pixel wide or tall, these scale factors are used to cap the maximum
            // value of coverage that is used. In other words it is the coverage that is
            // used in the interior of the rect after the ramp.
            fragBuilder->codeAppend("float scaleW = min(1.0, 2.0*insetW/spanW);");
            fragBuilder->codeAppend("float scaleH = min(1.0, 2.0*insetH/spanH);");
            // Compute the coverage for the rect's width
            fragBuilder->codeAppendf("vec2 offset = %s.xy - %s.xy;", positionVary.fsIn(),
                                     rectEdgeVary.fsIn());
            fragBuilder->codeAppendf("float perpDot = abs(offset.x * %s.w - offset.y * %s.z);",
                                     rectEdgeVary.fsIn(), rectEdgeVary.fsIn());

            if (args.fDistanceVectorName) {
                fragBuilder->codeAppendf("float widthDistance = %s.x - perpDot;",
                                         widthHeightVary.fsIn());
            }

            fragBuilder->codeAppendf(
                    "float coverage = scaleW*clamp((%s.x-perpDot)/spanW, 0.0, 1.0);",
                    widthHeightVary.fsIn());
            // Compute the coverage for the rect's height and merge with the width
            fragBuilder->codeAppendf("perpDot = abs(dot(offset, %s.zw));", rectEdgeVary.fsIn());

            if (args.fDistanceVectorName) {
                fragBuilder->codeAppendf("float heightDistance = %s.y - perpDot;",
                                         widthHeightVary.fsIn());
            }

            fragBuilder->codeAppendf(
                    "coverage = coverage*scaleH*clamp((%s.y-perpDot)/spanH, 0.0, 1.0);",
                    widthHeightVary.fsIn());

            fragBuilder->codeAppendf("%s = vec4(coverage);", args.fOutputCoverage);

            if (args.fDistanceVectorName) {
                fragBuilder->codeAppend("// Calculating distance vector\n");
                fragBuilder->codeAppend("vec2 dvAxis;");
                fragBuilder->codeAppend("float dvLength;");

                fragBuilder->codeAppend("if (heightDistance < widthDistance) {");
                fragBuilder->codeAppendf("    dvAxis = %s.zw;", rectEdgeVary.fsIn());
                fragBuilder->codeAppend("     dvLength = heightDistance;");
                fragBuilder->codeAppend("} else {");
                fragBuilder->codeAppendf("    dvAxis = vec2(-%s.w, %s.z);", rectEdgeVary.fsIn(),
                                         rectEdgeVary.fsIn());
                fragBuilder->codeAppend("     dvLength = widthDistance;");
                fragBuilder->codeAppend("}");

                fragBuilder->codeAppend("float dvSign = sign(dot(offset, dvAxis));");
                fragBuilder->codeAppendf("%s = vec4(dvSign * dvAxis, dvLength, 0.0);",
                                         args.fDistanceVectorName);
            }
        }

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrShaderCaps&,
                           GrProcessorKeyBuilder* b) {
            b->add32(0x0);
        }

        void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                     FPCoordTransformIter&& transformIter) override {
            const RectGeometryProcessor& rgp = primProc.cast<RectGeometryProcessor>();
            this->setTransformDataHelper(rgp.fLocalMatrix, pdman, &transformIter);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;
    };

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new GLSLProcessor();
    }

private:
    SkMatrix fLocalMatrix;

    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInRectEdge;
    const Attribute* fInWidthHeight;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(RectGeometryProcessor);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> RectGeometryProcessor::TestCreate(GrProcessorTestData* d) {
    return sk_sp<GrGeometryProcessor>(new RectGeometryProcessor(GrTest::TestMatrix(d->fRandom)));
}
#endif

///////////////////////////////////////////////////////////////////////////////

class AnalyticRectOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    AnalyticRectOp(GrColor color, const SkMatrix& viewMatrix, const SkRect& rect,
                   const SkRect& croppedRect, const SkRect& bounds)
            : INHERITED(ClassID()), fViewMatrixIfUsingLocalCoords(viewMatrix) {
        SkPoint center = SkPoint::Make(rect.centerX(), rect.centerY());
        viewMatrix.mapPoints(&center, 1);
        SkScalar halfWidth = viewMatrix.mapRadius(SkScalarHalf(rect.width()));
        SkScalar halfHeight = viewMatrix.mapRadius(SkScalarHalf(rect.height()));
        SkVector downDir = viewMatrix.mapVector(0.0f, 1.0f);
        downDir.normalize();

        SkRect deviceSpaceCroppedRect = croppedRect;
        viewMatrix.mapRect(&deviceSpaceCroppedRect);

        fGeoData.emplace_back(
                Geometry{color, center, downDir, halfWidth, halfHeight, deviceSpaceCroppedRect});

        this->setBounds(bounds, HasAABloat::kYes, IsZeroArea::kNo);
    }

    const char* name() const override { return "AnalyticRectOp"; }

    SkString dumpInfo() const override {
        SkString string;
        for (int i = 0; i < fGeoData.count(); ++i) {
            string.appendf("Color: 0x%08x Rect [C:(%.2f, %.2f) D:<%.2f,%.3f> W/2:%.2f H/2:%.2f]\n",
                           fGeoData[i].fColor, fGeoData[i].fCenter.x(), fGeoData[i].fCenter.y(),
                           fGeoData[i].fDownDir.x(), fGeoData[i].fDownDir.y(),
                           fGeoData[i].fHalfWidth, fGeoData[i].fHalfHeight);
        }
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(fGeoData[0].fColor);
        *coverage = GrProcessorAnalysisCoverage::kSingleChannel;
    }

    void applyPipelineOptimizations(const PipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fGeoData[0].fColor);
        if (!optimizations.readsLocalCoords()) {
            fViewMatrixIfUsingLocalCoords.reset();
        }
    }

    void onPrepareDraws(Target* target) const override {
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        sk_sp<GrGeometryProcessor> gp(new RectGeometryProcessor(localMatrix));

        int instanceCount = fGeoData.count();
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(RectVertex));
        QuadHelper helper;
        RectVertex* verts =
                reinterpret_cast<RectVertex*>(helper.init(target, vertexStride, instanceCount));
        if (!verts) {
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& geom = fGeoData[i];

            GrColor color = geom.fColor;
            SkPoint center = geom.fCenter;
            SkVector downDir = geom.fDownDir;
            SkScalar halfWidth = geom.fHalfWidth;
            SkScalar halfHeight = geom.fHalfHeight;
            SkRect croppedRect = geom.fCroppedRect;

            SkVector rightDir;
            downDir.rotateCCW(&rightDir);

            verts[0].fPos = {croppedRect.fLeft, croppedRect.fTop};
            verts[0].fColor = color;
            verts[0].fCenter = center;
            verts[0].fDownDir = downDir;
            verts[0].fHalfWidth = halfWidth;
            verts[0].fHalfHeight = halfHeight;

            verts[1].fPos = {croppedRect.fRight, croppedRect.fTop};
            verts[1].fColor = color;
            verts[1].fCenter = center;
            verts[1].fDownDir = downDir;
            verts[1].fHalfWidth = halfWidth;
            verts[1].fHalfHeight = halfHeight;

            verts[2].fPos = {croppedRect.fRight, croppedRect.fBottom};
            verts[2].fColor = color;
            verts[2].fCenter = center;
            verts[2].fDownDir = downDir;
            verts[2].fHalfWidth = halfWidth;
            verts[2].fHalfHeight = halfHeight;

            verts[3].fPos = {croppedRect.fLeft, croppedRect.fBottom};
            verts[3].fColor = color;
            verts[3].fCenter = center;
            verts[3].fDownDir = downDir;
            verts[3].fHalfWidth = halfWidth;
            verts[3].fHalfHeight = halfHeight;

            verts += kVerticesPerQuad;
        }
        helper.recordDraw(target, gp.get(), this->pipeline());
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        AnalyticRectOp* that = t->cast<AnalyticRectOp>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (!fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        this->joinBounds(*that);
        return true;
    }

    struct Geometry {
        GrColor fColor;
        SkPoint fCenter;
        SkVector fDownDir;
        SkScalar fHalfWidth;
        SkScalar fHalfHeight;
        SkRect fCroppedRect;
    };

    SkMatrix fViewMatrixIfUsingLocalCoords;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrLegacyMeshDrawOp INHERITED;
};

std::unique_ptr<GrLegacyMeshDrawOp> GrAnalyticRectOp::Make(GrColor color,
                                                           const SkMatrix& viewMatrix,
                                                           const SkRect& rect,
                                                           const SkRect& croppedRect,
                                                           const SkRect& bounds) {
    return std::unique_ptr<GrLegacyMeshDrawOp>(
            new AnalyticRectOp(color, viewMatrix, rect, croppedRect, bounds));
}

#if GR_TEST_UTILS

DRAW_OP_TEST_DEFINE(AnalyticRectOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrColor color = GrRandomColor(random);
    SkRect rect = GrTest::TestSquare(random);
    SkRect croppedRect = GrTest::TestSquare(random);
    SkRect bounds = GrTest::TestSquare(random);
    return std::unique_ptr<GrLegacyMeshDrawOp>(
            new AnalyticRectOp(color, viewMatrix, rect, croppedRect, bounds));
}

#endif
