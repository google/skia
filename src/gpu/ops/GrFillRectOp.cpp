/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrFillRectOp.h"

#include "GrGeometryProcessor.h"
#include "GrMeshDrawOp.h"
#include "GrPaint.h"
#include "GrQuad.h"
#include "GrQuadPerEdgeAA.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"

namespace {

using VertexSpec = GrQuadPerEdgeAA::VertexSpec;
using ColorType = GrQuadPerEdgeAA::ColorType;

// NOTE: This info structure is intentionally modeled after GrTextureOps' Quad so that they can
// more easily be integrated together in the future.
class TransformedQuad {
public:
    TransformedQuad(const GrPerspQuad& deviceQuad, const GrPerspQuad& localQuad,
                    const SkPMColor4f& color, GrQuadAAFlags aaFlags)
            : fDeviceQuad(deviceQuad)
            , fLocalQuad(localQuad)
            , fColor(color)
            , fAAFlags(aaFlags) {}

    const GrPerspQuad& deviceQuad() const { return fDeviceQuad; }
    const GrPerspQuad& localQuad() const { return fLocalQuad; }
    const SkPMColor4f& color() const { return fColor; }
    GrQuadAAFlags aaFlags() const { return fAAFlags; }

    void setColor(const SkPMColor4f& color) { fColor = color; }

    SkString dumpInfo(int index) const {
        SkString str;
        str.appendf("%d: Color: [%.2f, %.2f, %.2f, %.2f], Edge AA: l%u_t%u_r%u_b%u, \n"
                    "  device quad: [(%.2f, %2.f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), "
                    "(%.2f, %.2f, %.2f)],\n"
                    "  local quad: [(%.2f, %2.f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), "
                    "(%.2f, %.2f, %.2f)]\n",
                    index, fColor.fR, fColor.fG, fColor.fB, fColor.fA,
                    (uint32_t) (fAAFlags & GrQuadAAFlags::kLeft),
                    (uint32_t) (fAAFlags & GrQuadAAFlags::kTop),
                    (uint32_t) (fAAFlags & GrQuadAAFlags::kRight),
                    (uint32_t) (fAAFlags & GrQuadAAFlags::kBottom),
                    fDeviceQuad.x(0), fDeviceQuad.y(0), fDeviceQuad.w(0),
                    fDeviceQuad.x(1), fDeviceQuad.y(1), fDeviceQuad.w(1),
                    fDeviceQuad.x(2), fDeviceQuad.y(2), fDeviceQuad.w(2),
                    fDeviceQuad.x(3), fDeviceQuad.y(3), fDeviceQuad.w(3),
                    fLocalQuad.x(0), fLocalQuad.y(0), fLocalQuad.w(0),
                    fLocalQuad.x(1), fLocalQuad.y(1), fLocalQuad.w(1),
                    fLocalQuad.x(2), fLocalQuad.y(2), fLocalQuad.w(2),
                    fLocalQuad.x(3), fLocalQuad.y(3), fLocalQuad.w(3));
        return str;
    }
private:
    // NOTE: The TransformedQuad does not store the types for device and local. The owning op tracks
    // the most general type for device and local across all of its merged quads.
    GrPerspQuad fDeviceQuad; // In device space, allowing rects to be combined across view matrices
    GrPerspQuad fLocalQuad; // Original rect transformed by its local matrix
    SkPMColor4f fColor;
    GrQuadAAFlags fAAFlags;
};

// A GeometryProcessor for rendering TransformedQuads using the vertex attributes from
// GrQuadPerEdgeAA. This is similar to the TextureGeometryProcessor of GrTextureOp except that it
// handles full GrPaints.
class QuadPerEdgeAAGeometryProcessor : public GrGeometryProcessor {
public:

    static sk_sp<GrGeometryProcessor> Make(const VertexSpec& spec) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(spec));
    }

    const char* name() const override { return "QuadPerEdgeAAGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // The attributes' key includes the device and local quad types implicitly since those
        // types decide the vertex attribute size
        b->add32(fAttrs.getKey());
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps& caps) const override {
        class GLSLProcessor : public GrGLSLGeometryProcessor {
        public:
            void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& proc,
                         FPCoordTransformIter&& transformIter) override {
                const auto& gp = proc.cast<QuadPerEdgeAAGeometryProcessor>();
                if (gp.fAttrs.hasLocalCoords()) {
                    this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
                }
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                const auto& gp = args.fGP.cast<QuadPerEdgeAAGeometryProcessor>();
                args.fVaryingHandler->emitAttributes(gp);
                gpArgs->fPositionVar = gp.fAttrs.positions().asShaderVar();

                if (gp.fAttrs.hasLocalCoords()) {
                    this->emitTransforms(args.fVertBuilder,
                                         args.fVaryingHandler,
                                         args.fUniformHandler,
                                         gp.fAttrs.localCoords().asShaderVar(),
                                         args.fFPCoordTransformHandler);
                }

                gp.fAttrs.emitColor(args, "paintColor");
                gp.fAttrs.emitCoverage(args, "aaDist");
            }
        };
        return new GLSLProcessor;
    }

private:
    QuadPerEdgeAAGeometryProcessor(const VertexSpec& spec)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fAttrs(spec) {
        SkASSERT(spec.hasVertexColors());
        this->setVertexAttributes(fAttrs.attributes(), fAttrs.attributeCount());
    }

    GrQuadPerEdgeAA::GPAttributes fAttrs;

    typedef GrGeometryProcessor INHERITED;
};

class FillRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
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

        // Analyze the paint to see if it is compatible with scissor-clearing
        SkPMColor4f color = paint.getColor4f();
        // Only non-null if the paint can be turned into a clear, it can be a local pointer since
        // the op ctor consumes the value right away if it's provided
        SkPMColor4f* clearColor = nullptr;
        if (paint.isTrivial() || paint.isConstantBlendedColor(&color)) {
            clearColor = &color;
        }

        return Helper::FactoryHelper<FillRectOp>(context, std::move(paint), clearColor, aaType,
                edgeAA, stencilSettings, deviceQuad, deviceQuadType, localQuad, localQuadType);
    }

    // Analysis of the GrPaint to determine the const blend color must be done before, passing
    // nullptr for constBlendColor disables all scissor-clear optimizations (must keep the
    // paintColor argument because it is assumed by the GrSimpleMeshDrawOpHelper). Similarly, aaType
    // is passed to Helper in the initializer list, so incongruities between aaType and edgeFlags
    // must be resolved prior to calling this constructor.
    FillRectOp(Helper::MakeArgs args, SkPMColor4f paintColor, const SkPMColor4f* constBlendColor,
               GrAAType aaType, GrQuadAAFlags edgeFlags, const GrUserStencilSettings* stencil,
               const GrPerspQuad& deviceQuad, GrQuadType deviceQuadType,
               const GrPerspQuad& localQuad, GrQuadType localQuadType)
            : INHERITED(ClassID())
            , fHelper(args, aaType, stencil)
            , fDeviceQuadType(static_cast<unsigned>(deviceQuadType))
            , fLocalQuadType(static_cast<unsigned>(localQuadType)) {
        if (constBlendColor) {
            // The GrPaint is compatible with clearing, and the constant blend color overrides the
            // paint color (although in most cases they are probably the same)
            paintColor = *constBlendColor;
            // However, just because the paint is compatible, the device quad must also be a rect
            // that is non-AA (AA aligned with pixel bounds should have already been turned into
            // non-AA).
            fClearCompatible = deviceQuadType == GrQuadType::kRect && aaType == GrAAType::kNone;
        } else {
            // Paint isn't clear compatible
            fClearCompatible = false;
        }

        fWideColor = !SkPMColor4fFitsInBytes(paintColor);

        // The color stored with the quad is the clear color if a scissor-clear is decided upon
        // when executing the op.
        fQuads.emplace_back(deviceQuad, localQuad, paintColor, edgeFlags);
        this->setBounds(deviceQuad.bounds(), HasAABloat(aaType == GrAAType::kCoverage),
                        IsZeroArea::kNo);
    }

    const char* name() const override { return "FillRectOp"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        return fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# draws: %d\n", fQuads.count());
        str.appendf("Clear compatible: %u\n", static_cast<bool>(fClearCompatible));
        str.appendf("Device quad type: %u, local quad type: %u\n",
                    fDeviceQuadType, fLocalQuadType);
        str += fHelper.dumpInfo();
        for (int i = 0; i < fQuads.count(); i++) {
            str += fQuads[i].dumpInfo(i);

        }
        str += INHERITED::dumpInfo();
        return str;
    }
#endif

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        // Initialize aggregate color analysis with the first quad's color (which always exists)
        SkASSERT(fQuads.count() > 0);
        GrProcessorAnalysisColor quadColors(fQuads[0].color());
        // Then combine the colors of any additional quads (e.g. from MakeSet)
        for (int i = 1; i < fQuads.count(); ++i) {
            quadColors = GrProcessorAnalysisColor::Combine(quadColors, fQuads[i].color());
        }
        auto result = fHelper.xpRequiresDstTexture(
                caps, clip, GrProcessorAnalysisCoverage::kSingleChannel, &quadColors);
        // If there is a constant color after analysis, that means all of the quads should be set
        // to the same color (even if they started out with different colors).
        SkPMColor4f colorOverride;
        if (quadColors.isConstant(&colorOverride)) {
            for (int i = 0; i < fQuads.count(); ++i) {
                fQuads[i].setColor(colorOverride);
            }
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
    // FIXME(reviewer): better to just make addQuad public?
    friend std::unique_ptr<GrDrawOp> GrFillRectOp::MakeSet(GrContext* context, GrPaint&& paint,
            GrAAType aaType, const SkMatrix& viewMatrix,
            const GrRenderTargetContext::QuadSetEntry quads[], int quadCount,
            const GrUserStencilSettings* stencilSettings);

   void onPrepareDraws(Target* target) override {
        TRACE_EVENT0("skia", TRACE_FUNC);

        using Domain = GrQuadPerEdgeAA::Domain;
        static constexpr SkRect kEmptyDomain = SkRect::MakeEmpty();

        VertexSpec vertexSpec(this->deviceQuadType(),
                              fWideColor ? ColorType::kHalf : ColorType::kByte,
                              this->localQuadType(), fHelper.usesLocalCoords(), Domain::kNo,
                              fHelper.aaType());

        sk_sp<GrGeometryProcessor> gp = QuadPerEdgeAAGeometryProcessor::Make(vertexSpec);
        size_t vertexSize = gp->vertexStride();

        const GrBuffer* vbuffer;
        int vertexOffsetInBuffer = 0;

        // Fill the allocated vertex data
        void* vdata = target->makeVertexSpace(vertexSize, fQuads.count() * 4, &vbuffer,
                                              &vertexOffsetInBuffer);
        if (!vdata) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        // vertices pointer advances through vdata based on Tessellate's return value
        void* vertices = vdata;
        for (int i = 0; i < fQuads.count(); ++i) {
            const auto& q = fQuads[i];
            vertices = GrQuadPerEdgeAA::Tessellate(vertices, vertexSpec, q.deviceQuad(), q.color(),
                                                   q.localQuad(), kEmptyDomain, q.aaFlags());
        }

        // Configure the mesh for the vertex data
        GrMesh* mesh;
        if (fQuads.count() > 1) {
            mesh = target->allocMesh(GrPrimitiveType::kTriangles);
            sk_sp<const GrBuffer> ibuffer = target->resourceProvider()->refQuadIndexBuffer();
            if (!ibuffer) {
                SkDebugf("Could not allocate quad indices\n");
                return;
            }
            mesh->setIndexedPatterned(ibuffer.get(), 6, 4, fQuads.count(),
                                      GrResourceProvider::QuadCountOfQuadBuffer());
        } else {
            mesh = target->allocMesh(GrPrimitiveType::kTriangleStrip);
            mesh->setNonIndexedNonInstanced(4);
        }
        mesh->setVertexData(vbuffer, vertexOffsetInBuffer);

        auto pipe = fHelper.makePipeline(target);
        target->draw(std::move(gp), pipe.fPipeline, pipe.fFixedDynamicState, mesh);
   }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        TRACE_EVENT0("skia", TRACE_FUNC);
        const auto* that = t->cast<FillRectOp>();

        // Unlike most users of the draw op helper, this op can merge none-aa and coverage-aa
        // draw ops together, so pass true as the last argument.
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds(), true)) {
            return CombineResult::kCannotCombine;
        }

        // If the processor sets are compatible, the two ops are always compatible; it just needs
        // to adjust the state of the op to be the more general quad and aa types of the two ops.

        // The GrQuadType enum is ordered such that higher values are more general quad types
        if (that->fDeviceQuadType > fDeviceQuadType) {
            fDeviceQuadType = that->fDeviceQuadType;
        }
        if (that->fLocalQuadType > fLocalQuadType) {
            fLocalQuadType = that->fLocalQuadType;
        }
        fClearCompatible &= that->fClearCompatible;
        fWideColor |= that->fWideColor;

        // The helper stores the aa type, but isCompatible(with true arg) allows the two ops' aa
        // types to be none and coverage, in which case this op's aa type must be lifted to coverage
        // so that quads with no aa edges can be batched with quads that have some/all edges aa'ed.
        if (fHelper.aaType() == GrAAType::kNone && that->fHelper.aaType() == GrAAType::kCoverage) {
            fHelper.setAAType(GrAAType::kCoverage);
        }

        fQuads.push_back_n(that->fQuads.count(), that->fQuads.begin());
        return CombineResult::kMerged;
    }

    // Similar to onCombineIfPossible, but adds a quad assuming its op would have been compatible.
    // But since it's avoiding the op list management, it must update the op's bounds. This is only
    // used with quad sets, which uses the same view matrix for each quad so this assumes that the
    // device quad type of the new quad is the same as the op's.
    void addQuad(TransformedQuad&& quad, GrQuadType localQuadType, GrAAType aaType) {
        SkASSERT(quad.deviceQuad().quadType() <= this->deviceQuadType());

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

        // The new quad's local coordinates could differ
        if (localQuadType > this->localQuadType()) {
            fLocalQuadType = static_cast<unsigned>(localQuadType);
        }

        // clear compatible won't need to be updated, since device quad type and paint is the same,
        // but this quad has a new color, so maybe update wide color
        fWideColor |= !SkPMColor4fFitsInBytes(quad.color());

        // Update the bounds and add the quad to this op's storage
        SkRect newBounds = this->bounds();
        newBounds.joinPossiblyEmptyRect(quad.deviceQuad().bounds());
        this->setBounds(newBounds, HasAABloat(fHelper.aaType() == GrAAType::kCoverage),
                        IsZeroArea::kNo);
        fQuads.push_back(std::move(quad));
    }

    GrQuadType deviceQuadType() const { return static_cast<GrQuadType>(fDeviceQuadType); }
    GrQuadType localQuadType() const { return static_cast<GrQuadType>(fLocalQuadType); }

    Helper fHelper;
    SkSTArray<1, TransformedQuad, true> fQuads;

    // While we always store full GrPerspQuads in memory, if the type is known to be simpler we can
    // optimize our geometry generation.
    unsigned fDeviceQuadType: 2;
    unsigned fLocalQuadType: 2;
    unsigned fWideColor: 1;

    // True if fQuad produced by a rectangle-preserving view matrix, is pixel aligned or non-AA,
    // and its paint is a constant blended color.
    unsigned fClearCompatible: 1;

    typedef GrMeshDrawOp INHERITED;
};

} // anonymous namespace

namespace GrFillRectOp {

std::unique_ptr<GrDrawOp> Make(GrContext* context,
                               GrPaint&& paint,
                               GrAAType aaType,
                               GrQuadAAFlags edgeAA,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect,
                               const GrUserStencilSettings* stencilSettings) {
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad(rect, viewMatrix), GrQuadTypeForTransformedRect(viewMatrix),
                            GrPerspQuad(rect, SkMatrix::I()), GrQuadType::kRect);
}

std::unique_ptr<GrDrawOp> MakeWithLocalMatrix(GrContext* context,
                                              GrPaint&& paint,
                                              GrAAType aaType,
                                              GrQuadAAFlags edgeAA,
                                              const SkMatrix& viewMatrix,
                                              const SkMatrix& localMatrix,
                                              const SkRect& rect,
                                              const GrUserStencilSettings* stencilSettings) {
    GrQuadType localQuadType = GrQuadTypeForTransformedRect(localMatrix);
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad(rect, viewMatrix), GrQuadTypeForTransformedRect(viewMatrix),
                            GrPerspQuad(rect, localMatrix), localQuadType);
}

std::unique_ptr<GrDrawOp> MakeWithLocalRect(GrContext* context,
                                            GrPaint&& paint,
                                            GrAAType aaType,
                                            GrQuadAAFlags edgeAA,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect,
                                            const SkRect& localRect,
                                            const GrUserStencilSettings* stencilSettings) {
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad(rect, viewMatrix), GrQuadTypeForTransformedRect(viewMatrix),
                            GrPerspQuad(localRect, SkMatrix::I()), GrQuadType::kRect);
}

std::unique_ptr<GrDrawOp> MakeSet(GrContext* context,
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
            quads[0].fAAFlags, stencilSettings, GrPerspQuad(quads[0].fRect, viewMatrix),
            deviceQuadType, GrPerspQuad(quads[0].fRect, quads[0].fLocalMatrix),
            GrQuadTypeForTransformedRect(quads[0].fLocalMatrix));
    auto* fillRects = op->cast<FillRectOp>();

    // Accumulate remaining quads similar to onCombineIfPossible() without creating an op
    for (int i = 1; i < cnt; ++i) {
        GrPerspQuad deviceQuad(quads[i].fRect, viewMatrix);

        GrAAType resolvedAA;
        GrQuadAAFlags resolvedEdgeFlags;
        GrResolveAATypeForQuad(aaType, quads[i].fAAFlags, deviceQuad, deviceQuadType,
                               &resolvedAA, &resolvedEdgeFlags);

        fillRects->addQuad({ deviceQuad, GrPerspQuad(quads[i].fRect, quads[i].fLocalMatrix),
                             quads[i].fColor, resolvedEdgeFlags },
                           GrQuadTypeForTransformedRect(quads[i].fLocalMatrix), resolvedAA);
    }

    return op;
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
                return GrFillRectOp::MakeWithLocalMatrix(context, std::move(paint), aaType, aaFlags,
                                                         viewMatrix, localMatrix, rect, stencil);
            }
        } else {
            // Pass local rect directly
            SkRect localRect = GrTest::TestRect(random);
            return GrFillRectOp::MakeWithLocalRect(context, std::move(paint), aaType, aaFlags,
                                                   viewMatrix, rect, localRect, stencil);
        }
    } else {
        // The simplest constructor
        return GrFillRectOp::Make(context, std::move(paint), aaType, aaFlags, viewMatrix, rect,
                                  stencil);
    }
}

#endif
