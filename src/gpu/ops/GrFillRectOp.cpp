/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrFillRectOp.h"

#include "GrCaps.h"
#include "GrGeometryProcessor.h"
#include "GrMeshDrawOp.h"
#include "GrPaint.h"
#include "GrQuad.h"
#include "GrQuadPerEdgeAA.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "SkGr.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

namespace {

using VertexSpec = GrQuadPerEdgeAA::VertexSpec;
using ColorType = GrQuadPerEdgeAA::ColorType;

#ifdef SK_DEBUG
static SkString dump_quad_info(int index, const GrPerspQuad& deviceQuad,
                               const GrPerspQuad& localQuad, const SkPMColor4f& color,
                               GrQuadAAFlags aaFlags) {
    SkString str;
    str.appendf("%d: Color: [%.2f, %.2f, %.2f, %.2f], Edge AA: l%u_t%u_r%u_b%u, \n"
                "  device quad: [(%.2f, %2.f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), "
                "(%.2f, %.2f, %.2f)],\n"
                "  local quad: [(%.2f, %2.f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), "
                "(%.2f, %.2f, %.2f)]\n",
                index, color.fR, color.fG, color.fB, color.fA,
                (uint32_t) (aaFlags & GrQuadAAFlags::kLeft),
                (uint32_t) (aaFlags & GrQuadAAFlags::kTop),
                (uint32_t) (aaFlags & GrQuadAAFlags::kRight),
                (uint32_t) (aaFlags & GrQuadAAFlags::kBottom),
                deviceQuad.x(0), deviceQuad.y(0), deviceQuad.w(0),
                deviceQuad.x(1), deviceQuad.y(1), deviceQuad.w(1),
                deviceQuad.x(2), deviceQuad.y(2), deviceQuad.w(2),
                deviceQuad.x(3), deviceQuad.y(3), deviceQuad.w(3),
                localQuad.x(0), localQuad.y(0), localQuad.w(0),
                localQuad.x(1), localQuad.y(1), localQuad.w(1),
                localQuad.x(2), localQuad.y(2), localQuad.w(2),
                localQuad.x(3), localQuad.y(3), localQuad.w(3));
    return str;
}
#endif

class FillRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          GrAAType aaType,
                                          GrQuadAAFlags edgeAA,
                                          const GrUserStencilSettings* stencilSettings,
                                          const GrPerspQuad& deviceQuad,
                                          GrQuadType deviceQuadType,
                                          const GrPerspQuad& localQuad,
                                          GrQuadType localQuadType) {
        // Clean up deviations between aaType and edgeAA
        GrResolveAATypeForQuad(aaType, edgeAA, deviceQuad, deviceQuadType, &aaType, &edgeAA);
        return Helper::FactoryHelper<FillRectOp>(context, std::move(paint), aaType, edgeAA,
                stencilSettings, deviceQuad, deviceQuadType, localQuad, localQuadType);
    }

    // aaType is passed to Helper in the initializer list, so incongruities between aaType and
    // edgeFlags must be resolved prior to calling this constructor.
    FillRectOp(Helper::MakeArgs args, SkPMColor4f paintColor, GrAAType aaType,
               GrQuadAAFlags edgeFlags, const GrUserStencilSettings* stencil,
               const GrPerspQuad& deviceQuad, GrQuadType deviceQuadType,
               const GrPerspQuad& localQuad, GrQuadType localQuadType)
            : INHERITED(ClassID())
            , fHelper(args, aaType, stencil) {
        // The color stored with the quad is the clear color if a scissor-clear is decided upon
        // when executing the op.
        fDeviceQuads.push_back(deviceQuad, deviceQuadType, { paintColor, edgeFlags });

        if (!fHelper.isTrivial()) {
            // Conservatively keep track of the local coordinates; it may be that the paint doesn't
            // need them after analysis is finished. If the paint is known to be solid up front they
            // can be skipped entirely.
            fLocalQuads.push_back(localQuad, localQuadType);
        }
        this->setBounds(deviceQuad.bounds(deviceQuadType),
                        HasAABloat(aaType == GrAAType::kCoverage), IsZeroArea::kNo);
    }

    const char* name() const override { return "FillRectOp"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        return fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# draws: %u\n", this->quadCount());
        str.appendf("Device quad type: %u, local quad type: %u\n",
                    (uint32_t) fDeviceQuads.quadType(), (uint32_t) fLocalQuads.quadType());
        str += fHelper.dumpInfo();
        GrPerspQuad device, local;
        for (int i = 0; i < this->quadCount(); i++) {
            device = fDeviceQuads[i];
            const ColorAndAA& info = fDeviceQuads.metadata(i);
            if (!fHelper.isTrivial()) {
                local = fLocalQuads[i];
            }
            str += dump_quad_info(i, device, local, info.fColor, info.fAAFlags);
        }
        str += INHERITED::dumpInfo();
        return str;
    }
#endif

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrFSAAType fsaaType, GrClampType clampType) override {
        // Initialize aggregate color analysis with the first quad's color (which always exists)
        SkASSERT(this->quadCount() > 0);
        GrProcessorAnalysisColor quadColors(fDeviceQuads.metadata(0).fColor);
        // Then combine the colors of any additional quads (e.g. from MakeSet)
        for (int i = 1; i < this->quadCount(); ++i) {
            quadColors = GrProcessorAnalysisColor::Combine(quadColors,
                                                           fDeviceQuads.metadata(i).fColor);
            if (quadColors.isUnknown()) {
                // No point in accumulating additional starting colors, combining cannot make it
                // less unknown.
                break;
            }
        }

        // If the AA type is coverage, it will be a single value per pixel; if it's not coverage AA
        // then the coverage is always 1.0, so specify kNone for more optimal blending.
        GrProcessorAnalysisCoverage coverage = fHelper.aaType() == GrAAType::kCoverage ?
                GrProcessorAnalysisCoverage::kSingleChannel :
                GrProcessorAnalysisCoverage::kNone;
        auto result = fHelper.finalizeProcessors(
                caps, clip, fsaaType, clampType, coverage, &quadColors);
        // If there is a constant color after analysis, that means all of the quads should be set
        // to the same color (even if they started out with different colors).
        SkPMColor4f colorOverride;
        if (quadColors.isConstant(&colorOverride)) {
            fColorType = GrQuadPerEdgeAA::MinColorType(colorOverride, clampType, caps);
            for (int i = 0; i < this->quadCount(); ++i) {
                fDeviceQuads.metadata(i).fColor = colorOverride;
            }
        } else {
            // Otherwise compute the color type needed as the max over all quads.
            fColorType = ColorType::kNone;
            for (int i = 0; i < this->quadCount(); ++i) {
                SkPMColor4f* color = &fDeviceQuads.metadata(i).fColor;
                fColorType = SkTMax(fColorType,
                                    GrQuadPerEdgeAA::MinColorType(*color, clampType, caps));
            }
        }
        // Most SkShaders' FPs multiply their calculated color by the paint color or alpha. We want
        // to use ColorType::kNone to optimize out that multiply. However, if there are no color
        // FPs then were really writing a special shader for white rectangles and not saving any
        // multiples. So in that case use bytes to avoid the extra shader (and possibly work around
        // an ANGLE issue: crbug.com/942565).
        if (fColorType == ColorType::kNone && !result.hasColorFragmentProcessor()) {
            fColorType = ColorType::kByte;
        }

        return result;
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        // Since the AA type of the whole primitive is kept consistent with the per edge AA flags
        // the helper's fixed function flags are appropriate.
        return fHelper.fixedFunctionFlags();
    }

    DEFINE_OP_CLASS_ID

private:
    // For GrFillRectOp::MakeSet's use of addQuad
    friend std::unique_ptr<GrDrawOp> GrFillRectOp::MakeSet(
            GrRecordingContext*,
            GrPaint&&,
            GrAAType, const SkMatrix& viewMatrix,
            const GrRenderTargetContext::QuadSetEntry quads[], int quadCount,
            const GrUserStencilSettings*);

    void onPrepareDraws(Target* target) override {
        TRACE_EVENT0("skia", TRACE_FUNC);

        using Domain = GrQuadPerEdgeAA::Domain;
        static constexpr SkRect kEmptyDomain = SkRect::MakeEmpty();

        VertexSpec vertexSpec(fDeviceQuads.quadType(), fColorType, fLocalQuads.quadType(),
                              fHelper.usesLocalCoords(), Domain::kNo, fHelper.aaType(),
                              fHelper.compatibleWithCoverageAsAlpha());
        // Make sure that if the op thought it was a solid color, the vertex spec does not use
        // local coords.
        SkASSERT(!fHelper.isTrivial() || !fHelper.usesLocalCoords());

        sk_sp<GrGeometryProcessor> gp = GrQuadPerEdgeAA::MakeProcessor(vertexSpec);
        size_t vertexSize = gp->vertexStride();

        sk_sp<const GrBuffer> vbuffer;
        int vertexOffsetInBuffer = 0;

        // Fill the allocated vertex data
        void* vdata = target->makeVertexSpace(
                vertexSize, this->quadCount() * vertexSpec.verticesPerQuad(),
                &vbuffer, &vertexOffsetInBuffer);
        if (!vdata) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        // vertices pointer advances through vdata based on Tessellate's return value
        void* vertices = vdata;
        if (fHelper.isTrivial()) {
            SkASSERT(fLocalQuads.count() == 0); // No local coords, so send an ignored dummy quad
            static const GrPerspQuad kIgnoredLocal(SkRect::MakeEmpty());

            for (int i = 0; i < this->quadCount(); ++i) {
                const ColorAndAA& info = fDeviceQuads.metadata(i);
                vertices = GrQuadPerEdgeAA::Tessellate(vertices, vertexSpec, fDeviceQuads[i],
                        info.fColor, kIgnoredLocal, kEmptyDomain, info.fAAFlags);
            }
        } else {
            SkASSERT(fLocalQuads.count() == fDeviceQuads.count());
            for (int i = 0; i < this->quadCount(); ++i) {
                const ColorAndAA& info = fDeviceQuads.metadata(i);
                vertices = GrQuadPerEdgeAA::Tessellate(vertices, vertexSpec, fDeviceQuads[i],
                        info.fColor, fLocalQuads[i], kEmptyDomain, info.fAAFlags);
            }
        }

        // Configure the mesh for the vertex data
        GrMesh* mesh = target->allocMeshes(1);
        if (!GrQuadPerEdgeAA::ConfigureMeshIndices(target, mesh, vertexSpec, this->quadCount())) {
            SkDebugf("Could not allocate indices\n");
            return;
        }
        mesh->setVertexData(std::move(vbuffer), vertexOffsetInBuffer);
        target->recordDraw(std::move(gp), mesh);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        TRACE_EVENT0("skia", TRACE_FUNC);
        const auto* that = t->cast<FillRectOp>();

        if ((fHelper.aaType() == GrAAType::kCoverage ||
             that->fHelper.aaType() == GrAAType::kCoverage) &&
            this->quadCount() + that->quadCount() > GrQuadPerEdgeAA::kNumAAQuadsInIndexBuffer) {
            // This limit on batch size seems to help on Adreno devices
            return CombineResult::kCannotCombine;
        }

        // Unlike most users of the draw op helper, this op can merge none-aa and coverage-aa draw
        // ops together, so pass true as the last argument.
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds(), true)) {
            return CombineResult::kCannotCombine;
        }

        // If the paints were compatible, the trivial/solid-color state should be the same
        SkASSERT(fHelper.isTrivial() == that->fHelper.isTrivial());

        // If the processor sets are compatible, the two ops are always compatible; it just needs to
        // adjust the state of the op to be the more general quad and aa types of the two ops and
        // then concatenate the per-quad data.
        fColorType = SkTMax(fColorType, that->fColorType);

        // The helper stores the aa type, but isCompatible(with true arg) allows the two ops' aa
        // types to be none and coverage, in which case this op's aa type must be lifted to coverage
        // so that quads with no aa edges can be batched with quads that have some/all edges aa'ed.
        if (fHelper.aaType() == GrAAType::kNone && that->fHelper.aaType() == GrAAType::kCoverage) {
            fHelper.setAAType(GrAAType::kCoverage);
        }

        fDeviceQuads.concat(that->fDeviceQuads);
        if (!fHelper.isTrivial()) {
            fLocalQuads.concat(that->fLocalQuads);
        }
        return CombineResult::kMerged;
    }

    // Similar to onCombineIfPossible, but adds a quad assuming its op would have been compatible.
    // But since it's avoiding the op list management, it must update the op's bounds. This is only
    // used with quad sets, which uses the same view matrix for each quad so this assumes that the
    // device quad type of the new quad is the same as the op's.
    void addQuad(const GrPerspQuad& deviceQuad, const GrPerspQuad& localQuad,
                 GrQuadType localQuadType, const SkPMColor4f& color, GrQuadAAFlags edgeAA,
                 GrAAType aaType) {
        // The new quad's aa type should be the same as the first quad's or none, except when the
        // first quad's aa type was already downgraded to none, in which case the stored type must
        // be lifted to back to the requested type.
        if (aaType != fHelper.aaType()) {
            if (aaType != GrAAType::kNone) {
                // Original quad was downgraded to non-aa, lift back up to this quad's required type
                SkASSERT(fHelper.aaType() == GrAAType::kNone);
                fHelper.setAAType(aaType);
            }
            // else the new quad could have been downgraded but the other quads can't be, so don't
            // reset the op's accumulated aa type.
        }

        // Update the bounds and add the quad to this op's storage
        SkRect newBounds = this->bounds();
        newBounds.joinPossiblyEmptyRect(deviceQuad.bounds(fDeviceQuads.quadType()));
        this->setBounds(newBounds, HasAABloat(fHelper.aaType() == GrAAType::kCoverage),
                        IsZeroArea::kNo);
        fDeviceQuads.push_back(deviceQuad, fDeviceQuads.quadType(), { color, edgeAA });
        if (!fHelper.isTrivial()) {
            fLocalQuads.push_back(localQuad, localQuadType);
        }
    }

    int quadCount() const {
        // Sanity check that the parallel arrays for quad properties all have the same size
        SkASSERT(fDeviceQuads.count() == fLocalQuads.count() ||
                 (fLocalQuads.count() == 0 && fHelper.isTrivial()));
        return fDeviceQuads.count();
    }

    struct ColorAndAA {
        SkPMColor4f fColor;
        GrQuadAAFlags fAAFlags;
    };

    Helper fHelper;
    GrTQuadList<ColorAndAA> fDeviceQuads;
    // No metadata attached to the local quads; this list is empty when local coords are not needed.
    GrQuadList fLocalQuads;

    ColorType fColorType;

    typedef GrMeshDrawOp INHERITED;
};

} // anonymous namespace

namespace GrFillRectOp {

std::unique_ptr<GrDrawOp> MakePerEdge(GrRecordingContext* context,
                                      GrPaint&& paint,
                                      GrAAType aaType,
                                      GrQuadAAFlags edgeAA,
                                      const SkMatrix& viewMatrix,
                                      const SkRect& rect,
                                      const GrUserStencilSettings* stencilSettings) {
    GrQuadType dstQuadType = GrQuadTypeForTransformedRect(viewMatrix);
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad::MakeFromRect(rect, viewMatrix),  dstQuadType,
                            GrPerspQuad(rect), GrQuadType::kRect);
}

std::unique_ptr<GrDrawOp> MakePerEdgeWithLocalMatrix(GrRecordingContext* context,
                                                     GrPaint&& paint,
                                                     GrAAType aaType,
                                                     GrQuadAAFlags edgeAA,
                                                     const SkMatrix& viewMatrix,
                                                     const SkMatrix& localMatrix,
                                                     const SkRect& rect,
                                                     const GrUserStencilSettings* stencilSettings) {
    GrQuadType dstQuadType = GrQuadTypeForTransformedRect(viewMatrix);
    GrQuadType localQuadType = GrQuadTypeForTransformedRect(localMatrix);
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad::MakeFromRect(rect, viewMatrix), dstQuadType,
                            GrPerspQuad::MakeFromRect(rect, localMatrix), localQuadType);
}

std::unique_ptr<GrDrawOp> MakePerEdgeWithLocalRect(GrRecordingContext* context,
                                                   GrPaint&& paint,
                                                   GrAAType aaType,
                                                   GrQuadAAFlags edgeAA,
                                                   const SkMatrix& viewMatrix,
                                                   const SkRect& rect,
                                                   const SkRect& localRect,
                                                   const GrUserStencilSettings* stencilSettings) {
    GrQuadType dstQuadType = GrQuadTypeForTransformedRect(viewMatrix);
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad::MakeFromRect(rect, viewMatrix), dstQuadType,
                            GrPerspQuad(localRect), GrQuadType::kRect);
}

std::unique_ptr<GrDrawOp> MakePerEdgeQuad(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          GrAAType aaType,
                                          GrQuadAAFlags edgeAA,
                                          const SkMatrix& viewMatrix,
                                          const SkPoint quad[4],
                                          const SkPoint localQuad[4],
                                          const GrUserStencilSettings* stencilSettings) {
    GrQuadType deviceType = GrQuadTypeForPoints(quad, viewMatrix);
    GrQuadType localType = GrQuadTypeForPoints(localQuad ? localQuad : quad, SkMatrix::I());
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad::MakeFromSkQuad(quad, viewMatrix), deviceType,
                            GrPerspQuad::MakeFromSkQuad(localQuad ? localQuad : quad,
                                                        SkMatrix::I()), localType);
}

std::unique_ptr<GrDrawOp> MakeSet(GrRecordingContext* context,
                                  GrPaint&& paint,
                                  GrAAType aaType,
                                  const SkMatrix& viewMatrix,
                                  const GrRenderTargetContext::QuadSetEntry quads[],
                                  int cnt,
                                  const GrUserStencilSettings* stencilSettings) {
    // First make a draw op for the first quad in the set
    SkASSERT(cnt > 0);
    GrQuadType deviceQuadType = GrQuadTypeForTransformedRect(viewMatrix);

    paint.setColor4f(quads[0].fColor);
    std::unique_ptr<GrDrawOp> op = FillRectOp::Make(context, std::move(paint), aaType,
            quads[0].fAAFlags, stencilSettings,
            GrPerspQuad::MakeFromRect(quads[0].fRect, viewMatrix), deviceQuadType,
            GrPerspQuad::MakeFromRect(quads[0].fRect, quads[0].fLocalMatrix),
            GrQuadTypeForTransformedRect(quads[0].fLocalMatrix));
    auto* fillRects = op->cast<FillRectOp>();

    // Accumulate remaining quads similar to onCombineIfPossible() without creating an op
    for (int i = 1; i < cnt; ++i) {
        GrPerspQuad deviceQuad = GrPerspQuad::MakeFromRect(quads[i].fRect, viewMatrix);

        GrAAType resolvedAA;
        GrQuadAAFlags resolvedEdgeFlags;
        GrResolveAATypeForQuad(aaType, quads[i].fAAFlags, deviceQuad, deviceQuadType,
                               &resolvedAA, &resolvedEdgeFlags);

        fillRects->addQuad(deviceQuad,
                           GrPerspQuad::MakeFromRect(quads[i].fRect, quads[i].fLocalMatrix),
                           GrQuadTypeForTransformedRect(quads[i].fLocalMatrix), quads[i].fColor,
                           resolvedEdgeFlags,resolvedAA);
    }

    return op;
}

std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                               GrPaint&& paint,
                               GrAAType aaType,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect,
                               const GrUserStencilSettings* stencil) {
    return MakePerEdge(context, std::move(paint), aaType,
            aaType == GrAAType::kCoverage ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone,
            viewMatrix, rect, stencil);
}

std::unique_ptr<GrDrawOp> MakeWithLocalMatrix(GrRecordingContext* context,
                                              GrPaint&& paint,
                                              GrAAType aaType,
                                              const SkMatrix& viewMatrix,
                                              const SkMatrix& localMatrix,
                                              const SkRect& rect,
                                              const GrUserStencilSettings* stencil) {
    return MakePerEdgeWithLocalMatrix(context, std::move(paint), aaType,
            aaType == GrAAType::kCoverage ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone,
            viewMatrix, localMatrix, rect, stencil);
}

std::unique_ptr<GrDrawOp> MakeWithLocalRect(GrRecordingContext* context,
                                            GrPaint&& paint,
                                            GrAAType aaType,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect,
                                            const SkRect& localRect,
                                            const GrUserStencilSettings* stencil) {
    return MakePerEdgeWithLocalRect(context, std::move(paint), aaType,
            aaType == GrAAType::kCoverage ? GrQuadAAFlags::kAll : GrQuadAAFlags::kNone,
            viewMatrix, rect, localRect, stencil);
}

} // namespace GrFillRectOp

#if GR_TEST_UTILS

#include "GrDrawOpTest.h"
#include "SkGr.h"

GR_DRAW_OP_TEST_DEFINE(FillRectOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkRect rect = GrTest::TestRect(random);

    GrAAType aaType = GrAAType::kNone;
    if (random->nextBool()) {
        aaType = (fsaaType == GrFSAAType::kUnifiedMSAA) ? GrAAType::kMSAA : GrAAType::kCoverage;
    }
    const GrUserStencilSettings* stencil = random->nextBool() ? nullptr
                                                              : GrGetRandomStencil(random, context);

    GrQuadAAFlags aaFlags = GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kLeft : GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kTop : GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kRight : GrQuadAAFlags::kNone;
    aaFlags |= random->nextBool() ? GrQuadAAFlags::kBottom : GrQuadAAFlags::kNone;

    if (random->nextBool()) {
        if (random->nextBool()) {
            if (random->nextBool()) {
                // Local matrix with a set op
                uint32_t extraQuadCt = random->nextRangeU(1, 4);
                SkTArray<GrRenderTargetContext::QuadSetEntry> quads(extraQuadCt + 1);
                quads.push_back(
                        {rect, SkPMColor4f::FromBytes_RGBA(SkColorToPremulGrColor(random->nextU())),
                         GrTest::TestMatrixInvertible(random), aaFlags});
                for (uint32_t i = 0; i < extraQuadCt; ++i) {
                    GrQuadAAFlags aaFlags = GrQuadAAFlags::kNone;
                    aaFlags |= random->nextBool() ? GrQuadAAFlags::kLeft : GrQuadAAFlags::kNone;
                    aaFlags |= random->nextBool() ? GrQuadAAFlags::kTop : GrQuadAAFlags::kNone;
                    aaFlags |= random->nextBool() ? GrQuadAAFlags::kRight : GrQuadAAFlags::kNone;
                    aaFlags |= random->nextBool() ? GrQuadAAFlags::kBottom : GrQuadAAFlags::kNone;

                    quads.push_back(
                        {GrTest::TestRect(random),
                         SkPMColor4f::FromBytes_RGBA(SkColorToPremulGrColor(random->nextU())),
                         GrTest::TestMatrixInvertible(random), aaFlags});
                }

                return GrFillRectOp::MakeSet(context, std::move(paint), aaType, viewMatrix,
                                             quads.begin(), quads.count(), stencil);
            } else {
                // Single local matrix
                SkMatrix localMatrix = GrTest::TestMatrixInvertible(random);
                return GrFillRectOp::MakePerEdgeWithLocalMatrix(context, std::move(paint), aaType,
                                                                aaFlags, viewMatrix, localMatrix,
                                                                rect, stencil);
            }
        } else {
            // Pass local rect directly
            SkRect localRect = GrTest::TestRect(random);
            return GrFillRectOp::MakePerEdgeWithLocalRect(context, std::move(paint), aaType,
                                                          aaFlags, viewMatrix, rect, localRect,
                                                          stencil);
        }
    } else {
        // The simplest constructor
        return GrFillRectOp::MakePerEdge(context, std::move(paint), aaType, aaFlags, viewMatrix,
                                         rect, stencil);
    }
}

#endif
