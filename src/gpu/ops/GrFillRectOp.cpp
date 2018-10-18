/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMeshDrawOp.h"
#include "GrPaint.h"
#include "GrQuad.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "SkMatrix.h"
#include "SkRect.h"

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

class FillRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelperWithStencil;

public:
    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          GrPaint&& paint,
                                          GrAAType aaType,
                                          GrQuadAAFlags edgeAA,
                                          GrUserStencilSettings* stencilSettings,
                                          const GrPerspQuad& deviceQuad,
                                          GrQuadType deviceQuadType,
                                          const GrPerspQuad& srcQuad,
                                          GrQuadType srcQuadType) {
        // Clean up deviations between aaType and edgeAA
        GrResolveAATypeForQuad(aaType, edgeAA, deviceQuad, &deviceQuadType, &aaType, &edgeAA);

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
    FillRectOp(Helper::MakeArgs args, GrColor paintColor, GrColor* constBlendColor, GrAAType aaType,
               GrQuadAAFlags edgeFlags, GrUserStencilSettings* stencilSettings,
               const GrPerspQuad& deviceQuad, GrQuadType deviceQuadType,
               const GrPerspQuad& srcQuad, GrQuadType srcQuadType)
            : INHERITED(ClassID())
            , fHelper(args, aaType, stencilSettings)
            , fDeviceQuadType(deviceQuadType)
            , fSrcQuadType(srcQuadType) {
        if (constBlendColor) {
            // The GrPaint is compatible with clearing, and effectively overrides the paint's color
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
    }

    void onPrepareDraws(Target* target) override {
        // FIXME actual prepare the geometry
    }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        // FIXME actually implement this logic
        return CombineResult::kCannotCombine;
    }

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
                               GrUserStencilSettings* stencilSettings,
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
                                              GrUserStencilSettings* stencilSettings,
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
                                            GrUserStencilSettings* stencilSettings,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& rect,
                                            const SkRect& localRect) {
    return FillRectOp::Make(context, std::move(paint), aaType, edgeAA, stencilSettings,
                            GrPerspQuad(rect, viewMatrix), GrQuadTypeForTransformedRect(viewMatrix),
                            GrPerspQuad(localRect, SkMatrix::I()), GrQuadType::kRect_QuadType);
}

} // namespace GrFillRectOp
