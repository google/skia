/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/TestRectOp.h"

#include "src/core/SkPointPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

namespace {
class GP : public GrGeometryProcessor {
public:
    GP(const SkMatrix& localMatrix, bool wideColor) : GrGeometryProcessor(kTestRectOp_ClassID), fLocalMatrix(localMatrix) {
            fInColor = MakeColorAttribute("color", wideColor);
            this->setVertexAttributes(&fInPosition, 3);
    }
    const char* name() const override { return "test"; }
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLGP : public GrGLSLGeometryProcessor {
            void setData(const GrGLSLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& pp,
                         FPCoordTransformIter&& iter) override {
                const auto& gp =  pp.cast<GP>();
                this->setTransformDataHelper(gp.fLocalMatrix, pdman, &iter);
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                const auto& gp =  args.fGP.cast<GP>();
                args.fVaryingHandler->emitAttributes(gp);
                GrGLSLVarying colorVarying(kHalf4_GrSLType);
                args.fVaryingHandler->addVarying("color", &colorVarying, GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
                args.fVertBuilder->codeAppendf("%s = %s;", colorVarying.vsOut(), gp.fInColor.name());
                args.fFragBuilder->codeAppendf("%s = %s;", args.fOutputColor,
                                               colorVarying.fsIn());
                args.fFragBuilder->codeAppendf("%s = half4(1);", args.fOutputCoverage);
                this->writeOutputPosition(args.fVertBuilder, gpArgs, gp.fInPosition.name());
                this->emitTransforms(args.fVertBuilder, args.fVaryingHandler, args.fUniformHandler, gp.fInLocalCoords.asShaderVar(), gp.fLocalMatrix, args.fFPCoordTransformHandler);
            }
        };
        return new GLSLGP();
    }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

private:
    Attribute fInPosition = {"inPosition", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    Attribute fInLocalCoords = {"inLocalCoords", kFloat2_GrVertexAttribType, kFloat2_GrSLType};
    Attribute fInColor;
    SkMatrix fLocalMatrix;
};
}  // anonymous namespace

namespace sk_gpu_test {

std::unique_ptr<GrDrawOp> TestRectOp::Make(GrRecordingContext* context, GrPaint&& paint, const SkRect& drawRect, const SkRect& localRect, const SkMatrix& localM) {
    auto* pool = context->priv().opMemoryPool();
    const auto* caps = context->priv().caps();
    return pool->allocate<TestRectOp>(caps, std::move(paint), drawRect, localRect, localM);
}

std::unique_ptr<GrDrawOp> TestRectOp::Make(GrRecordingContext* context, std::unique_ptr<GrFragmentProcessor> fp, const SkRect& drawRect, const SkRect& localRect, const SkMatrix& localM) {
    GrPaint paint;
    paint.addColorFragmentProcessor(std::move(fp));
    return Make(context, std::move(paint), drawRect, localRect, localM);
}

std::unique_ptr<GrDrawOp> TestRectOp::Make(GrRecordingContext* context, GrPaint&& paint, const SkRect& drawRect) {
    return Make(context, std::move(paint), drawRect, drawRect, SkMatrix::I());
}

GrProcessorSet::Analysis TestRectOp::finalize(
        const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
        GrClampType clampType) {
    return fProcessorSet.finalize(
            GrProcessorAnalysisColor::Opaque::kYes, GrProcessorAnalysisCoverage::kSingleChannel, clip,
            &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps, clampType, &fColor);
}


TestRectOp::TestRectOp(const GrCaps* caps, GrPaint&& paint, const SkRect& drawRect, const SkRect& localRect, const SkMatrix& localMatrix)
        : GrMeshDrawOp(ClassID())
        , fDrawRect(drawRect)
        , fLocalRect(localRect)
        , fLocalMatrix(localMatrix)
        , fColor(paint.getColor4f())
        , fProcessorSet(std::move(paint)) {
    fWideColor = !fColor.fitsInBytes() && caps->halfFloatVertexAttributeSupport();
    fGeometryProcessor = sk_make_sp<GP>(fLocalMatrix, fWideColor);
    this->setBounds(drawRect.makeSorted(), HasAABloat::kNo, IsHairline::kNo);
}

void TestRectOp::onPrepareDraws(Target* target) {
    QuadHelper helper(target, fGeometryProcessor->vertexStride(), 1);
    GrVertexWriter writer{helper.vertices()};
    auto pos = GrVertexWriter::TriStripFromRect(fDrawRect);
    auto local = GrVertexWriter::TriStripFromRect(fLocalRect);
    GrVertexColor color(fColor, fWideColor);
    writer.writeQuad(pos, local, color);
    helper.recordDraw(target, fGeometryProcessor);
}

void TestRectOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    flushState->executeDrawsAndUploadsForMeshDrawOp(
            this, chainBounds, std::move(fProcessorSet));
}

}
