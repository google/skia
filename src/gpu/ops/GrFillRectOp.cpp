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

// NOTE: This info structure is intentionally modeled after GrTextureOps' Quad so that they can
// more easily be integrated together in the future.
class TransformedQuad {
public:
    TransformedQuad(const GrPerspQuad& deviceQuad, const GrPerspQuad& srcQuad,
                    const GrColor& color, GrQuadAAFlags aaFlags)
            : fDeviceQuad(deviceQuad)
            , fSrcQuad(srcQuad)
            , fColor(color)
            , fAAFlags(static_cast<unsigned>(aaFlags)) {
        SkASSERT(fAAFlags == static_cast<unsigned>(aaFlags));
    }

    const GrPerspQuad& deviceQuad() const { return fDeviceQuad; }
    const GrPerspQuad& srcQuad() const { return fSrcQuad; }
    const GrColor& color() const { return fColor; }
    GrQuadAAFlags aaFlags() const { return static_cast<GrQuadAAFlags>(fAAFlags); }

    void setColor(const GrColor& color) { fColor = color; }

    SkString dumpInfo(int index) const {
        GrQuadAAFlags edges = static_cast<GrQuadAAFlags>(fAAFlags);
        SkString str;
        str.appendf("%d: Color: 0x%08x, Edge AA: l%u_t%u_r%u_b%u, \n"
                    "  device quad: [(%.2f, %2.f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), "
                    "(%.2f, %.2f, %.2f)],\n"
                    "  local quad: [(%.2f, %2.f, %.2f), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f), "
                    "(%.2f, %.2f, %.2f)]\n",
                    index, fColor, edges & GrQuadAAFlags::kLeft,  edges & GrQuadAAFlags::kTop,
                    edges & GrQuadAAFlags::kRight, edges & GrQuadAAFlags::kBottom,
                    fDeviceQuad.x(0), fDeviceQuad.y(0), fDeviceQuad.w(0),
                    fDeviceQuad.x(1), fDeviceQuad.y(1), fDeviceQuad.w(1),
                    fDeviceQuad.x(2), fDeviceQuad.y(2), fDeviceQuad.w(2),
                    fDeviceQuad.x(3), fDeviceQuad.y(3), fDeviceQuad.w(3),
                    fSrcQuad.x(0), fSrcQuad.y(0), fSrcQuad.w(0),
                    fSrcQuad.x(1), fSrcQuad.y(1), fSrcQuad.w(1),
                    fSrcQuad.x(2), fSrcQuad.y(2), fSrcQuad.w(2),
                    fSrcQuad.x(3), fSrcQuad.y(3), fSrcQuad.w(3));
        return str;
    }
private:
    // NOTE: The TransformedQuad does not store the types for device and src. The owning op tracks
    // the most general type for device and src across all of its merged quads.
    GrPerspQuad fDeviceQuad; // In device space, allowing rects to be combined across view matrices
    GrPerspQuad fSrcQuad; // Original rect transformed by its local matrix
    GrColor fColor;
    unsigned fAAFlags: 4;
};

// A GeometryProcessor for rendering TransformedQuads using the vertex attributes from
// GrQuadPerEdgeAA. This is similar to the TextureGeometryProcessor of GrTextureOp except that it
// handles full GrPaints.
class QuadPerEdgeAAGeometryProcessor : public GrGeometryProcessor {
public:

    static sk_sp<GrGeometryProcessor> Make(sk_sp<GrColorSpaceXform> paintColorSpaceXform,
                                           int posDim, int localPosDim, GrAAType aa) {
        return sk_sp<QuadPerEdgeAAGeometryProcessor>(new QuadPerEdgeAAGeometryProcessor(
                std::move(paintColorSpaceXform), posDim, localPosDim, aa));
    }

    const char* name() const override { return "QuadPerEdgeAAGeometryProcessor"; }

    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(GrColorSpaceXform::XformKey(fPaintColorSpaceXform.get()));
        // The attributes' key includes the device and src quad types implicitly since those
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
                fPaintColorSpaceXformHelper.setData(pdman, gp.fPaintColorSpaceXform.get());
            }

        private:
            void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
                const auto& gp = args.fGP.cast<QuadPerEdgeAAGeometryProcessor>();
                fPaintColorSpaceXformHelper.emitCode(
                        args.fUniformHandler, gp.fPaintColorSpaceXform.get(),
                        kVertex_GrShaderFlag);
                if (!gp.fAttrs.needsPerspectiveInterpolation()) {
                    args.fVaryingHandler->setNoPerspective();
                }
                args.fVaryingHandler->emitAttributes(gp);
                gpArgs->fPositionVar = gp.fAttrs.positions().asShaderVar();

                if (gp.fAttrs.hasLocalCoords()) {
                    this->emitTransforms(args.fVertBuilder,
                                         args.fVaryingHandler,
                                         args.fUniformHandler,
                                         gp.fAttrs.localCoords().asShaderVar(),
                                         args.fFPCoordTransformHandler);
                }

                // FIXME this may not handle SkColor's correctly... see DefaultGeoProc
                gp.fAttrs.emitColor(args, &fPaintColorSpaceXformHelper, "paintColor");
                gp.fAttrs.emitCoverage(args, "aaDist");
            }

            GrGLSLColorSpaceXformHelper fPaintColorSpaceXformHelper;
        };
        return new GLSLProcessor;
    }

private:
    QuadPerEdgeAAGeometryProcessor(sk_sp<GrColorSpaceXform> paintColorSpaceXform,
                                   int posDim, int localPosDim, GrAAType aa)
            : INHERITED(kQuadPerEdgeAAGeometryProcessor_ClassID)
            , fAttrs(posDim, localPosDim, /* vertex color */ true, aa, GrQuadPerEdgeAA::Domain::kNo)
            , fPaintColorSpaceXform(std::move(paintColorSpaceXform)) {
        this->setVertexAttributeCnt(fAttrs.vertexAttributeCount());
    }

    const Attribute& onVertexAttribute(int i) const override {
        // This GP never uses the domain attribute available for vertices, so don't include in
        // in the list of options
        return IthInitializedAttribute(i, fAttrs.positions(), fAttrs.colors(), fAttrs.localCoords(),
                                       fAttrs.edges(0), fAttrs.edges(1), fAttrs.edges(2),
                                       fAttrs.edges(3));
    }

    GrQuadPerEdgeAA::GPAttributes fAttrs;
    sk_sp<GrColorSpaceXform> fPaintColorSpaceXform;

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
                                          const GrPerspQuad& srcQuad,
                                          GrQuadType srcQuadType) {
        // Clean up deviations between aaType and edgeAA
        GrResolveAATypeForQuad(aaType, edgeAA, deviceQuad, deviceQuadType, &aaType, &edgeAA);

        // Analyze the paint to see if it is compatible with scissor-clearing
        GrColor color = paint.getColor();
        // Only non-null if the paint can be turned into a clear
        GrColor* clearColor = nullptr;
        if (paint.isTrivial() || paint.isConstantBlendedColor(&color)) {
            clearColor = &color;
        }

        return Helper::FactoryHelper<FillRectOp>(context, std::move(paint), clearColor,
                aaType, edgeAA, stencilSettings, deviceQuad, deviceQuadType, srcQuad, srcQuadType);
    }

    ~FillRectOp() override { }

    const char* name() const override { return "FillRectOp"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        return fHelper.visitProxies(func);
    }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("# draws: %d\n", fQuads.count());
        str.appendf("Clear compatible: %u\n", static_cast<bool>(fClearCompatible));
        str.appendf("Device quad type: %u, src quad type: %u\n",
                    static_cast<GrQuadType>(fDeviceQuadType), static_cast<GrQuadType>(fSrcQuadType));
        str += fHelper.dumpInfo();
        for (int i = 0; i < fQuads.count(); i++) {
            str += fQuads[i].dumpInfo(i);

        }
        str += INHERITED::dumpInfo();
        return str;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        // FIXME is finalize called before the ops are batched? or does this need to calculate the
        // modified color for each quad?
        GrColor color = fQuads[0].color();
        auto result = fHelper.xpRequiresDstTexture(
                caps, clip, GrProcessorAnalysisCoverage::kSingleChannel, &color);
        fQuads[0].setColor(color);
        return result;
    }

    FixedFunctionFlags fixedFunctionFlags() const override {
        // Since the AA type of the whole primitive is kept consistent with the per edge AA flags
        // the helper's fixed function flags are appropriate.
        return fHelper.fixedFunctionFlags();
    }

    DEFINE_OP_CLASS_ID

private:
    friend class ::GrOpMemoryPool;
    template <typename Op, typename... OpArgs>
    friend std::unique_ptr<GrDrawOp> GrSimpleMeshDrawOpHelper::FactoryHelper(GrContext*, GrPaint&&,
                                                                             OpArgs...);

    // Analysis of the GrPaint to determine the const blend color must be done before, passing
    // nullptr for constBlendColor disables all scissor-clear optimizations (must keep the
    // paintColor argument because it is assumed by the GrSimpleMeshDrawOpHelper). Similarly, aaType
    // is passed to Helper in the initializer list, so incongruities between aaType and edgeFlags
    // must be resolved prior to calling this constructor.
    FillRectOp(Helper::MakeArgs args, GrColor paintColor, const GrColor* constBlendColor,
               GrAAType aaType, GrQuadAAFlags edgeFlags, const GrUserStencilSettings* stencil,
               const GrPerspQuad& deviceQuad, GrQuadType deviceQuadType,
               const GrPerspQuad& srcQuad, GrQuadType srcQuadType)
            : INHERITED(ClassID())
            , fHelper(args, aaType, stencil)
            , fDeviceQuadType(static_cast<unsigned>(deviceQuadType))
            , fSrcQuadType(static_cast<unsigned>(srcQuadType)) {
        if (constBlendColor) {
            // The GrPaint is compatible with clearing, and the constant blend color overrides the
            // paint color (although in most cases they are probably the same)
            paintColor = *constBlendColor;
            // However, just because the paint is compatible, the device quad must also be a rect
            // that is non-AA (AA aligned with pixel bounds should have already been turned into
            // non-AA).
            fClearCompatible = deviceQuadType == GrQuadType::kRect_QuadType &&
                    aaType == GrAAType::kNone;
        } else {
            // Paint isn't clear compatible
            fClearCompatible = false;
        }

        // The color stored with the quad is the clear color if a scissor-clear is decided upon
        // when executing the op.
        fQuads.emplace_back(deviceQuad, srcQuad, paintColor, edgeFlags);
        this->setBounds(deviceQuad.bounds(), HasAABloat(aaType == GrAAType::kCoverage),
                        IsZeroArea::kNo);
    }

    template <int PosDim, int LocalPosDim, GrAA AA>
    void tess(void* v, const GrGeometryProcessor* gp) const {
        TRACE_EVENT0("skia", TRACE_FUNC);
        using Domain = GrQuadPerEdgeAA::Domain;
        using Vertex = GrQuadPerEdgeAA::Vertex<PosDim, GrColor, LocalPosDim, Domain::kNo, AA>;
        static constexpr SkRect kEmptyDomain = SkRect::MakeEmpty();
        SkASSERT(gp->debugOnly_vertexStride() == sizeof(Vertex));

        auto vertices = static_cast<Vertex*>(v);
        for (int i = 0; i < fQuads.count(); ++i) {
            const auto q = fQuads[i];
            GrQuadPerEdgeAA::Tessellate<Vertex>(vertices, q.deviceQuad(), q.color(), q.srcQuad(),
                                                kEmptyDomain, q.aaFlags());
            vertices += 4;
        }
    }

   void onPrepareDraws(Target* target) override {
        int posDim = this->deviceQuadType() == GrQuadType::kPerspective_QuadType ? 3 : 2;
        int srcDim = fHelper.usesLocalCoords() ?
                (this->srcQuadType() == GrQuadType::kPerspective_QuadType ? 3 : 2) : 0;
        // FIXME paint color xform?
        sk_sp<GrGeometryProcessor> gp = QuadPerEdgeAAGeometryProcessor::Make(
                nullptr, posDim, srcDim, fHelper.aaType());

// FIXME this is almost identical to GrTextureOp, barring the different template variables. Would
// be nice to find a way to reuse this pattern between the two ops
        using TessFn = decltype(&FillRectOp::tess<2, 2, GrAA::kNo>);
#define TESS_FN_AND_VERTEX_SIZE(DevDim, SrcDim, AA)                            \
    {                                                                          \
        &FillRectOp::tess<DevDim, SrcDim, AA>,                                 \
        sizeof(GrQuadPerEdgeAA::Vertex<DevDim, GrColor, SrcDim, GrQuadPerEdgeAA::Domain::kNo, AA>) \
    }
        static constexpr struct {
            TessFn fTessFn;
            size_t fVertexSize;
        } kTessFnsAndVertexSizes[] = {
                TESS_FN_AND_VERTEX_SIZE(2, 0, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(2, 0, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(3, 0, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(3, 0, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(2, 2, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(2, 2, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(3, 2, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(3, 2, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(2, 3, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(2, 3, GrAA::kYes),
                TESS_FN_AND_VERTEX_SIZE(3, 3, GrAA::kNo),
                TESS_FN_AND_VERTEX_SIZE(3, 3, GrAA::kYes),
        };
#undef TESS_FN_AND_VERTEX_SIZE
        int tessFnIdx = 0;
        tessFnIdx |= (GrAAType::kCoverage == fHelper.aaType()) ? 0x1 : 0x0;
        tessFnIdx |= (posDim == 3) ? 0x2 : 0x0;
        tessFnIdx += (srcDim == 3 ? 8 : (srcDim == 2 ? 4 : 0));

        size_t vertexSize = kTessFnsAndVertexSizes[tessFnIdx].fVertexSize;
        SkASSERT(vertexSize == gp->debugOnly_vertexStride());

        const GrBuffer* vbuffer;
        int vertexOffsetInBuffer = 0;

        // Fill the allocated vertex data
        void* vdata = target->makeVertexSpace(vertexSize, fQuads.count() * 4, &vbuffer,
                                        &vertexOffsetInBuffer);
        if (!vdata) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }
        (this->*(kTessFnsAndVertexSizes[tessFnIdx].fTessFn))(vdata, gp.get());

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
        // FIXME actually implement this logic
        return CombineResult::kCannotCombine;
    }

    GrQuadType deviceQuadType() const { return static_cast<GrQuadType>(fDeviceQuadType); }
    GrQuadType srcQuadType() const { return static_cast<GrQuadType>(fSrcQuadType); }

    Helper fHelper;
    SkSTArray<1, TransformedQuad, true> fQuads;

    // While we always store full GrPerspQuads in memory, if the type is known to be simpler we can
    // optimize our geometry generation.
    unsigned fDeviceQuadType: 2;
    unsigned fSrcQuadType: 2;

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
                               const GrUserStencilSettings* stencilSettings,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect) {
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad(rect, viewMatrix), GrQuadTypeForTransformedRect(viewMatrix),
                            GrPerspQuad(rect, SkMatrix::I()), GrQuadType::kRect_QuadType);
}

std::unique_ptr<GrDrawOp> MakeWithLocalMatrix(GrContext* context,
                                              GrPaint&& paint,
                                              GrAAType aaType,
                                              GrQuadAAFlags edgeAA,
                                              const GrUserStencilSettings* stencilSettings,
                                              const SkMatrix& viewMatrix,
                                              const SkMatrix& localMatrix,
                                              const SkRect& rect) {
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad(rect, viewMatrix), GrQuadTypeForTransformedRect(viewMatrix),
                            GrPerspQuad(rect, localMatrix), GrQuadTypeForTransformedRect(localMatrix));
}

std::unique_ptr<GrDrawOp> MakeWithLocalRect(GrContext* context,
                                            GrPaint&& paint,
                                            GrAAType aaType,
                                            GrQuadAAFlags edgeAA,
                                            const GrUserStencilSettings* stencilSettings,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect,
                                            const SkRect& localRect) {
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad(rect, viewMatrix), GrQuadTypeForTransformedRect(viewMatrix),
                            GrPerspQuad(localRect, SkMatrix::I()), GrQuadType::kRect_QuadType);
}

} // namespace GrFillRectOp


#include "GrRectOpFactory.h"

#ifdef USE_NEW_METHOD

namespace GrRectOpFactory {

std::unique_ptr<GrDrawOp> MakeAAFill(GrContext* context,
                                     GrPaint&& paint,
                                     const SkMatrix& viewMatrix,
                                     const SkRect& rect,
                                     const GrUserStencilSettings* stencilSettings) {
    return GrFillRectOp::Make(context, std::move(paint), GrAAType::kCoverage, GrQuadAAFlags::kAll,
                              stencilSettings, viewMatrix, rect);
}

std::unique_ptr<GrDrawOp> MakeAAFillWithLocalMatrix(GrContext* context,
                                                    GrPaint&& paint,
                                                    const SkMatrix& viewMatrix,
                                                    const SkMatrix& localMatrix,
                                                    const SkRect& rect) {
    return GrFillRectOp::MakeWithLocalMatrix(context, std::move(paint), GrAAType::kCoverage,
            GrQuadAAFlags::kAll, nullptr, viewMatrix, localMatrix, rect);
}

std::unique_ptr<GrDrawOp> MakeAAFillWithLocalRect(GrContext* context,
                                                  GrPaint&& paint,
                                                  const SkMatrix& viewMatrix,
                                                  const SkRect& rect,
                                                  const SkRect& localRect) {
    return GrFillRectOp::MakeWithLocalRect(context, std::move(paint), GrAAType::kCoverage,
            GrQuadAAFlags::kAll, nullptr, viewMatrix, rect, localRect);
}

/** Non-AA Fill - GrAAType must be either kNone or kMSAA. */

std::unique_ptr<GrDrawOp> MakeNonAAFill(GrContext* context,
                                        GrPaint&& paint,
                                        const SkMatrix& viewMatrix,
                                        const SkRect& rect,
                                        GrAAType aaType,
                                        const GrUserStencilSettings* stencilSettings) {
    // If aaType is kMSAA, the kNone flags will be resolved to all; if aaType is anything else the
    // kNone edge flags will force it to no anti aliasing.
    return GrFillRectOp::Make(context, std::move(paint), aaType, GrQuadAAFlags::kNone,
                              stencilSettings, viewMatrix, rect);
}

std::unique_ptr<GrDrawOp> MakeNonAAFillWithLocalMatrix(GrContext* context,
                                                       GrPaint&& paint,
                                                       const SkMatrix& viewMatrix,
                                                       const SkMatrix& localMatrix,
                                                       const SkRect& rect,
                                                       GrAAType aaType,
                                                       const GrUserStencilSettings* stencilSettings) {
    return GrFillRectOp::MakeWithLocalMatrix(context, std::move(paint), aaType, GrQuadAAFlags::kNone,
            stencilSettings, viewMatrix, localMatrix, rect);
}

std::unique_ptr<GrDrawOp> MakeNonAAFillWithLocalRect(GrContext* context,
                                                     GrPaint&& paint,
                                                     const SkMatrix& viewMatrix,
                                                     const SkRect& rect,
                                                     const SkRect& localRect,
                                                     GrAAType aaType) {
    return GrFillRectOp::MakeWithLocalRect(context, std::move(paint), aaType, GrQuadAAFlags::kNone,
            nullptr, viewMatrix, rect, localRect);
}

} // namespace GrRectOpFactory

#endif
