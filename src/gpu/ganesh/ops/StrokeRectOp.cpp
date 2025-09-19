/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/ops/StrokeRectOp.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkStrokeRec.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/base/SkAlignedStorage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkOnce.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/core/SkColorData.h"
#include "src/core/SkMatrixPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrBuffer.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDefaultGeoProcFactory.h"
#include "src/gpu/ganesh/GrDrawOpTest.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrSimpleMesh.h"
#include "src/gpu/ganesh/GrTestUtils.h"
#include "src/gpu/ganesh/geometry/GrQuad.h"
#include "src/gpu/ganesh/ops/FillRectOp.h"
#include "src/gpu/ganesh/ops/GrMeshDrawOp.h"
#include "src/gpu/ganesh/ops/GrSimpleMeshDrawOpHelper.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

class GrDstProxyView;
class GrGpuBuffer;
class GrSurfaceProxyView;
class SkArenaAlloc;
enum class GrXferBarrierFlags;

namespace skgpu::ganesh {
class SurfaceDrawContext;
}

using namespace skia_private;

namespace skgpu::ganesh::StrokeRectOp {

namespace {

// This emits line primitives for hairlines, so only support hairlines if allowed by caps. Otherwise
// we support all hairlines, bevels, and miters, but not round joins. Also, check whether the miter
// limit makes a miter join effectively beveled. If the miter is effectively beveled, it is only
// supported when using an AA stroke.
inline bool allowed_stroke(const GrCaps* caps, const SkStrokeRec& stroke, GrAA aa, bool* isMiter) {
    SkASSERT(stroke.getStyle() == SkStrokeRec::kStroke_Style ||
             stroke.getStyle() == SkStrokeRec::kHairline_Style);
    if (caps->avoidLineDraws() && stroke.isHairlineStyle()) {
        return false;
    }
    // For hairlines, make bevel and round joins appear the same as mitered ones.
    if (!stroke.getWidth()) {
        *isMiter = true;
        return true;
    }
    if (stroke.getJoin() == SkPaint::kBevel_Join) {
        *isMiter = false;
        return aa == GrAA::kYes; // bevel only supported with AA
    }
    if (stroke.getJoin() == SkPaint::kMiter_Join) {
        *isMiter = stroke.getMiter() >= SK_ScalarSqrt2;
        // Supported under non-AA only if it remains mitered
        return aa == GrAA::kYes || *isMiter;
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Non-AA Stroking
///////////////////////////////////////////////////////////////////////////////////////////////////

/*  create a triangle strip that strokes the specified rect. There are 8
    unique vertices, but we repeat the last 2 to close up. Alternatively we
    could use an indices array, and then only send 8 verts, but not sure that
    would be faster.
    */
void init_nonaa_stroke_rect_strip(SkPoint verts[10], const SkRect& rect, SkScalar width) {
    const SkScalar rad = SkScalarHalf(width);

    verts[0].set(rect.fLeft + rad, rect.fTop + rad);
    verts[1].set(rect.fLeft - rad, rect.fTop - rad);
    verts[2].set(rect.fRight - rad, rect.fTop + rad);
    verts[3].set(rect.fRight + rad, rect.fTop - rad);
    verts[4].set(rect.fRight - rad, rect.fBottom - rad);
    verts[5].set(rect.fRight + rad, rect.fBottom + rad);
    verts[6].set(rect.fLeft + rad, rect.fBottom - rad);
    verts[7].set(rect.fLeft - rad, rect.fBottom + rad);
    verts[8] = verts[0];
    verts[9] = verts[1];

    // TODO: we should be catching this higher up the call stack and just draw a single
    // non-AA rect
    if (2*rad >= rect.width()) {
        verts[0].fX = verts[2].fX = verts[4].fX = verts[6].fX = verts[8].fX = rect.centerX();
    }
    if (2*rad >= rect.height()) {
        verts[0].fY = verts[2].fY = verts[4].fY = verts[6].fY = verts[8].fY = rect.centerY();
    }
}

class NonAAStrokeRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "NonAAStrokeRectOp"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const SkMatrix& viewMatrix,
                            const SkRect& rect,
                            const SkStrokeRec& stroke,
                            GrAAType aaType) {
        bool isMiter;
        if (!allowed_stroke(context->priv().caps(), stroke, GrAA::kNo, &isMiter)) {
            return nullptr;
        }
        Helper::InputFlags inputFlags = Helper::InputFlags::kNone;
        // Depending on sub-pixel coordinates and the particular GPU, we may lose a corner of
        // hairline rects. We jam all the vertices to pixel centers to avoid this, but not
        // when MSAA is enabled because it can cause ugly artifacts.
        if (stroke.getStyle() == SkStrokeRec::kHairline_Style && aaType != GrAAType::kMSAA) {
            inputFlags |= Helper::InputFlags::kSnapVerticesToPixelCenters;
        }
        return Helper::FactoryHelper<NonAAStrokeRectOp>(context, std::move(paint), inputFlags,
                                                        viewMatrix, rect,
                                                        stroke, aaType);
    }

    NonAAStrokeRectOp(GrProcessorSet* processorSet, const SkPMColor4f& color,
                      Helper::InputFlags inputFlags, const SkMatrix& viewMatrix, const SkRect& rect,
                      const SkStrokeRec& stroke, GrAAType aaType)
            : INHERITED(ClassID())
            , fHelper(processorSet, aaType, inputFlags) {
        fColor = color;
        fViewMatrix = viewMatrix;
        fRect = rect;
        // Sort the rect for hairlines
        fRect.sort();
        fStrokeWidth = stroke.getWidth();

        SkScalar rad = SkScalarHalf(fStrokeWidth);
        SkRect bounds = rect;
        bounds.outset(rad, rad);

        // If our caller snaps to pixel centers then we have to round out the bounds
        if (inputFlags & Helper::InputFlags::kSnapVerticesToPixelCenters) {
            SkASSERT(!fStrokeWidth || aaType == GrAAType::kNone);
            viewMatrix.mapRect(&bounds);
            // We want to be consistent with how we snap non-aa lines. To match what we do in
            // GrGLSLVertexShaderBuilder, we first floor all the vertex values and then add half a
            // pixel to force us to pixel centers.
            bounds.setLTRB(SkScalarFloorToScalar(bounds.fLeft),
                           SkScalarFloorToScalar(bounds.fTop),
                           SkScalarFloorToScalar(bounds.fRight),
                           SkScalarFloorToScalar(bounds.fBottom));
            bounds.offset(0.5f, 0.5f);
            this->setBounds(bounds, HasAABloat::kNo, IsHairline::kNo);
        } else {
            HasAABloat aaBloat = (aaType == GrAAType::kNone) ? HasAABloat ::kNo : HasAABloat::kYes;
            this->setTransformedBounds(bounds, fViewMatrix, aaBloat,
                                       fStrokeWidth ? IsHairline::kNo : IsHairline::kYes);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        // This Op uses uniform (not vertex) color, so doesn't need to track wide color.
        return fHelper.finalizeProcessors(caps, clip, clampType, GrProcessorAnalysisCoverage::kNone,
                                          &fColor, nullptr);
    }

private:
    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&& clip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        GrGeometryProcessor* gp;
        {
            using namespace GrDefaultGeoProcFactory;
            Color color(fColor);
            LocalCoords::Type localCoordsType = fHelper.usesLocalCoords()
                                                        ? LocalCoords::kUsePosition_Type
                                                        : LocalCoords::kUnused_Type;
            gp = GrDefaultGeoProcFactory::Make(arena, color, Coverage::kSolid_Type, localCoordsType,
                                               fViewMatrix);
        }

        GrPrimitiveType primType = (fStrokeWidth > 0) ? GrPrimitiveType::kTriangleStrip
                                                      : GrPrimitiveType::kLineStrip;

        fProgramInfo = fHelper.createProgramInfo(caps, arena, writeView, usesMSAASurface,
                                                 std::move(clip), dstProxyView, gp, primType,
                                                 renderPassXferBarriers, colorLoadOp);
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        if (!fProgramInfo) {
            this->createProgramInfo(target);
        }

        size_t kVertexStride = fProgramInfo->geomProc().vertexStride();
        int vertexCount = kVertsPerHairlineRect;
        if (fStrokeWidth > 0) {
            vertexCount = kVertsPerStrokeRect;
        }

        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex;

        void* verts =
                target->makeVertexSpace(kVertexStride, vertexCount, &vertexBuffer, &firstVertex);

        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        SkPoint* vertex = reinterpret_cast<SkPoint*>(verts);

        if (fStrokeWidth > 0) {
            init_nonaa_stroke_rect_strip(vertex, fRect, fStrokeWidth);
        } else {
            // hairline
            vertex[0].set(fRect.fLeft, fRect.fTop);
            vertex[1].set(fRect.fRight, fRect.fTop);
            vertex[2].set(fRect.fRight, fRect.fBottom);
            vertex[3].set(fRect.fLeft, fRect.fBottom);
            vertex[4].set(fRect.fLeft, fRect.fTop);
        }

        fMesh = target->allocMesh();
        fMesh->set(std::move(vertexBuffer), vertexCount, firstVertex);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fMesh) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

#if defined(GPU_TEST_UTILS)
    SkString onDumpInfo() const override {
        return SkStringPrintf("Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                              "StrokeWidth: %.2f\n%s",
                              fColor.toBytes_RGBA(), fRect.fLeft, fRect.fTop, fRect.fRight,
                              fRect.fBottom, fStrokeWidth, fHelper.dumpInfo().c_str());
    }
#endif

    // TODO: override onCombineIfPossible

    Helper         fHelper;
    SkPMColor4f    fColor;
    SkMatrix       fViewMatrix;
    SkRect         fRect;
    SkScalar       fStrokeWidth;
    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;

    const static int kVertsPerHairlineRect = 5;
    const static int kVertsPerStrokeRect = 10;

    using INHERITED = GrMeshDrawOp;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// AA Stroking
///////////////////////////////////////////////////////////////////////////////////////////////////

SKGPU_DECLARE_STATIC_UNIQUE_KEY(gMiterIndexBufferKey);
SKGPU_DECLARE_STATIC_UNIQUE_KEY(gBevelIndexBufferKey);

bool stroke_dev_half_size_supported(SkVector devHalfStrokeSize) {
    // Since the horizontal and vertical strokes share internal corners, the coverage value at that
    // corner needs to be equal for the horizontal and vertical strokes both.
    //
    // The inner coverage values will be equal if the horizontal and vertical stroke widths are
    // equal (in which case innerCoverage is same for all sides of the rects) or if the horizontal
    // and vertical stroke widths are both greater than 1 (in which case innerCoverage will always
    // be 1). In actuality we allow them to be nearly-equal since differing by < 1/1000 will not be
    // visually detectable when the shape is already less than 1px in thickness.
    return SkScalarNearlyEqual(devHalfStrokeSize.fX, devHalfStrokeSize.fY) ||
           std::min(devHalfStrokeSize.fX, devHalfStrokeSize.fY) >= .5f;
}

bool compute_aa_rects(const GrCaps& caps,
                      SkRect* devOutside,
                      SkRect* devOutsideAssist,
                      SkRect* devInside,
                      bool* isDegenerate,
                      const SkMatrix& viewMatrix,
                      const SkRect& rect,
                      SkScalar strokeWidth,
                      bool miterStroke,
                      SkVector* devHalfStrokeSize) {
    SkVector devStrokeSize;
    if (strokeWidth > 0) {
        devStrokeSize = viewMatrix.mapVector({strokeWidth, strokeWidth});
        devStrokeSize.setAbs(devStrokeSize);
    } else {
        devStrokeSize.set(SK_Scalar1, SK_Scalar1);
    }

    const SkScalar dx = devStrokeSize.fX;
    const SkScalar dy = devStrokeSize.fY;
    const SkScalar rx = SkScalarHalf(dx);
    const SkScalar ry = SkScalarHalf(dy);

    devHalfStrokeSize->fX = rx;
    devHalfStrokeSize->fY = ry;

    SkRect devRect;
    viewMatrix.mapRect(&devRect, rect);

    // Clip our draw rect 1 full stroke width plus bloat outside the viewport. This avoids
    // interpolation precision issues with very large coordinates.
    const float m = caps.maxRenderTargetSize();
    const SkRect visibilityBounds = SkRect::MakeWH(m, m).makeOutset(dx + 1, dy + 1);
    if (!devRect.intersect(visibilityBounds)) {
        return false;
    }

    *devOutside = devRect;
    *devOutsideAssist = devRect;
    *devInside = devRect;

    devOutside->outset(rx, ry);
    devInside->inset(rx, ry);

    // If we have a degenerate stroking rect(ie the stroke is larger than inner rect) then we
    // make a degenerate inside rect to avoid double hitting.  We will also jam all of the points
    // together when we render these rects.
    SkScalar spare;
    {
        SkScalar w = devRect.width() - dx;
        SkScalar h = devRect.height() - dy;
        spare = std::min(w, h);
    }

    *isDegenerate = spare <= 0;
    if (*isDegenerate) {
        devInside->fLeft = devInside->fRight = devRect.centerX();
        devInside->fTop = devInside->fBottom = devRect.centerY();
    }

    // For bevel-stroke, use 2 SkRect instances(devOutside and devOutsideAssist)
    // to draw the outside of the octagon. Because there are 8 vertices on the outer
    // edge, while vertex number of inner edge is 4, the same as miter-stroke.
    if (!miterStroke) {
        devOutside->inset(0, ry);
        devOutsideAssist->outset(0, ry);
    }

    return true;
}

GrGeometryProcessor* create_aa_stroke_rect_gp(SkArenaAlloc* arena,
                                              bool usesMSAASurface,
                                              bool tweakAlphaForCoverage,
                                              const SkMatrix& viewMatrix,
                                              bool usesLocalCoords,
                                              bool wideColor) {
    using namespace GrDefaultGeoProcFactory;

    // When MSAA is enabled, we have to extend our AA bloats and interpolate coverage values outside
    // 0..1. We tell the gp in this case that coverage is an unclamped attribute so it will call
    // saturate(coverage) in the fragment shader.
    Coverage::Type coverageType = usesMSAASurface ? Coverage::kAttributeUnclamped_Type
                        : (!tweakAlphaForCoverage ? Coverage::kAttribute_Type
                                                  : Coverage::kSolid_Type);
    LocalCoords::Type localCoordsType =
        usesLocalCoords ? LocalCoords::kUsePosition_Type : LocalCoords::kUnused_Type;
    Color::Type colorType =
        wideColor ? Color::kPremulWideColorAttribute_Type: Color::kPremulGrColorAttribute_Type;

    return MakeForDeviceSpace(arena, colorType, coverageType, localCoordsType, viewMatrix);
}

class AAStrokeRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    // TODO support AA rotated stroke rects by copying around view matrices
    struct RectInfo {
        SkPMColor4f fColor;
        SkRect      fDevOutside;
        SkRect      fDevOutsideAssist;
        SkRect      fDevInside;
        SkVector    fDevHalfStrokeSize;
        bool        fDegenerate;
    };

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const SkMatrix& viewMatrix,
                            const SkRect& devOutside,
                            const SkRect& devInside,
                            const SkVector& devHalfStrokeSize) {
        if (!viewMatrix.rectStaysRect()) {
            // The AA op only supports axis-aligned rectangles
            return nullptr;
        }
        if (!stroke_dev_half_size_supported(devHalfStrokeSize)) {
            return nullptr;
        }
        return Helper::FactoryHelper<AAStrokeRectOp>(context, std::move(paint), viewMatrix,
                                                     devOutside, devInside, devHalfStrokeSize);
    }

    AAStrokeRectOp(GrProcessorSet* processorSet, const SkPMColor4f& color,
                   const SkMatrix& viewMatrix, const SkRect& devOutside, const SkRect& devInside,
                   const SkVector& devHalfStrokeSize)
            : INHERITED(ClassID())
            , fHelper(processorSet, GrAAType::kCoverage)
            , fViewMatrix(viewMatrix) {
        SkASSERT(!devOutside.isEmpty());
        SkASSERT(!devInside.isEmpty());

        fRects.emplace_back(RectInfo{color, devOutside, devOutside, devInside, devHalfStrokeSize, false});
        this->setBounds(devOutside, HasAABloat::kYes, IsHairline::kNo);
        fMiterStroke = true;
    }

    static GrOp::Owner Make(GrRecordingContext* context,
                            GrPaint&& paint,
                            const SkMatrix& viewMatrix,
                            const SkRect& rect,
                            const SkStrokeRec& stroke) {
        if (!viewMatrix.rectStaysRect()) {
            // The AA op only supports axis-aligned rectangles
            return nullptr;
        }
        bool isMiter;
        if (!allowed_stroke(context->priv().caps(), stroke, GrAA::kYes, &isMiter)) {
            return nullptr;
        }
        RectInfo info;
        if (!compute_aa_rects(*context->priv().caps(),
                              &info.fDevOutside,
                              &info.fDevOutsideAssist,
                              &info.fDevInside,
                              &info.fDegenerate,
                              viewMatrix,
                              rect,
                              stroke.getWidth(),
                              isMiter,
                              &info.fDevHalfStrokeSize)) {
            return nullptr;
        }
        if (!stroke_dev_half_size_supported(info.fDevHalfStrokeSize)) {
            return nullptr;
        }
        return Helper::FactoryHelper<AAStrokeRectOp>(context, std::move(paint), viewMatrix, info,
                                                     isMiter);
    }

    AAStrokeRectOp(GrProcessorSet* processorSet, const SkPMColor4f& color,
                   const SkMatrix& viewMatrix, const RectInfo& infoExceptColor, bool isMiter)
            : INHERITED(ClassID())
            , fHelper(processorSet, GrAAType::kCoverage)
            , fViewMatrix(viewMatrix) {
        fMiterStroke = isMiter;
        RectInfo& info = fRects.push_back(infoExceptColor);
        info.fColor = color;
        if (isMiter) {
            this->setBounds(info.fDevOutside, HasAABloat::kYes, IsHairline::kNo);
        } else {
            // The outer polygon of the bevel stroke is an octagon specified by the points of a
            // pair of overlapping rectangles where one is wide and the other is narrow.
            SkRect bounds = info.fDevOutside;
            bounds.joinPossiblyEmptyRect(info.fDevOutsideAssist);
            this->setBounds(bounds, HasAABloat::kYes, IsHairline::kNo);
        }
    }

    const char* name() const override { return "AAStrokeRect"; }

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrClampType clampType) override {
        return fHelper.finalizeProcessors(caps, clip, clampType,
                                          GrProcessorAnalysisCoverage::kSingleChannel,
                                          &fRects.back().fColor, &fWideColor);
    }

private:
    GrProgramInfo* programInfo() override { return fProgramInfo; }

    bool compatibleWithCoverageAsAlpha(bool usesMSAASurface) const {
        // When MSAA is enabled, we have to extend our AA bloats and interpolate coverage values
        // outside 0..1. This makes us incompatible with coverage as alpha.
        return !usesMSAASurface && fHelper.compatibleWithCoverageAsAlpha();
    }

    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&&,
                             const GrDstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override;

    void onPrepareDraws(GrMeshDrawTarget*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

#if defined(GPU_TEST_UTILS)
    SkString onDumpInfo() const override {
        SkString string;
        for (const auto& info : fRects) {
            string.appendf(
                    "Color: 0x%08x, ORect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                    "AssistORect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                    "IRect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], Degen: %d",
                    info.fColor.toBytes_RGBA(), info.fDevOutside.fLeft, info.fDevOutside.fTop,
                    info.fDevOutside.fRight, info.fDevOutside.fBottom, info.fDevOutsideAssist.fLeft,
                    info.fDevOutsideAssist.fTop, info.fDevOutsideAssist.fRight,
                    info.fDevOutsideAssist.fBottom, info.fDevInside.fLeft, info.fDevInside.fTop,
                    info.fDevInside.fRight, info.fDevInside.fBottom, info.fDegenerate);
        }
        string += fHelper.dumpInfo();
        return string;
    }
#endif

    static const int kMiterIndexCnt = 3 * 24;
    static const int kMiterVertexCnt = 16;
    static const int kNumMiterRectsInIndexBuffer = 256;

    static const int kBevelIndexCnt = 48 + 36 + 24;
    static const int kBevelVertexCnt = 24;
    static const int kNumBevelRectsInIndexBuffer = 256;

    static sk_sp<const GrGpuBuffer> GetIndexBuffer(GrResourceProvider*, bool miterStroke);

    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool miterStroke() const { return fMiterStroke; }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps&) override;

    void generateAAStrokeRectGeometry(VertexWriter& vertices,
                                      const SkPMColor4f& color,
                                      bool wideColor,
                                      const SkRect& devOutside,
                                      const SkRect& devOutsideAssist,
                                      const SkRect& devInside,
                                      bool miterStroke,
                                      bool degenerate,
                                      const SkVector& devHalfStrokeSize,
                                      bool usesMSAASurface) const;

    Helper         fHelper;
    STArray<1, RectInfo, true> fRects;
    SkMatrix       fViewMatrix;
    GrSimpleMesh*  fMesh = nullptr;
    GrProgramInfo* fProgramInfo = nullptr;
    bool           fMiterStroke;
    bool           fWideColor;

    using INHERITED = GrMeshDrawOp;
};

void AAStrokeRectOp::onCreateProgramInfo(const GrCaps* caps,
                                         SkArenaAlloc* arena,
                                         const GrSurfaceProxyView& writeView,
                                         bool usesMSAASurface,
                                         GrAppliedClip&& appliedClip,
                                         const GrDstProxyView& dstProxyView,
                                         GrXferBarrierFlags renderPassXferBarriers,
                                         GrLoadOp colorLoadOp) {

    GrGeometryProcessor* gp = create_aa_stroke_rect_gp(
            arena,
            usesMSAASurface,
            this->compatibleWithCoverageAsAlpha(usesMSAASurface),
            this->viewMatrix(),
            fHelper.usesLocalCoords(),
            fWideColor);
    if (!gp) {
        SkDebugf("Couldn't create GrGeometryProcessor\n");
        return;
    }

    fProgramInfo = fHelper.createProgramInfo(caps,
                                             arena,
                                             writeView,
                                             usesMSAASurface,
                                             std::move(appliedClip),
                                             dstProxyView,
                                             gp,
                                             GrPrimitiveType::kTriangles,
                                             renderPassXferBarriers,
                                             colorLoadOp);
}

void AAStrokeRectOp::onPrepareDraws(GrMeshDrawTarget* target) {

    if (!fProgramInfo) {
        this->createProgramInfo(target);
        if (!fProgramInfo) {
            return;
        }
    }

    int innerVertexNum = 4;
    int outerVertexNum = this->miterStroke() ? 4 : 8;
    int verticesPerInstance = (outerVertexNum + innerVertexNum) * 2;
    int indicesPerInstance = this->miterStroke() ? kMiterIndexCnt : kBevelIndexCnt;
    int instanceCount = fRects.size();
    int maxQuads = this->miterStroke() ? kNumMiterRectsInIndexBuffer : kNumBevelRectsInIndexBuffer;

    sk_sp<const GrGpuBuffer> indexBuffer =
            GetIndexBuffer(target->resourceProvider(), this->miterStroke());
    if (!indexBuffer) {
        SkDebugf("Could not allocate indices\n");
        return;
    }
    PatternHelper helper(target, GrPrimitiveType::kTriangles,
                         fProgramInfo->geomProc().vertexStride(), std::move(indexBuffer),
                         verticesPerInstance, indicesPerInstance, instanceCount, maxQuads);
    VertexWriter vertices{ helper.vertices() };
    if (!vertices) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    for (int i = 0; i < instanceCount; i++) {
        const RectInfo& info = fRects[i];
        this->generateAAStrokeRectGeometry(vertices,
                                           info.fColor,
                                           fWideColor,
                                           info.fDevOutside,
                                           info.fDevOutsideAssist,
                                           info.fDevInside,
                                           fMiterStroke,
                                           info.fDegenerate,
                                           info.fDevHalfStrokeSize,
                                           target->usesMSAASurface());
    }
    fMesh = helper.mesh();
}

void AAStrokeRectOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fProgramInfo || !fMesh) {
        return;
    }

    flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
    flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
    flushState->drawMesh(*fMesh);
}

sk_sp<const GrGpuBuffer> AAStrokeRectOp::GetIndexBuffer(GrResourceProvider* resourceProvider,
                                                        bool miterStroke) {
    if (miterStroke) {
        // clang-format off
        static const uint16_t gMiterIndices[] = {
            0 + 0, 1 + 0, 5 + 0, 5 + 0, 4 + 0, 0 + 0,
            1 + 0, 2 + 0, 6 + 0, 6 + 0, 5 + 0, 1 + 0,
            2 + 0, 3 + 0, 7 + 0, 7 + 0, 6 + 0, 2 + 0,
            3 + 0, 0 + 0, 4 + 0, 4 + 0, 7 + 0, 3 + 0,

            0 + 4, 1 + 4, 5 + 4, 5 + 4, 4 + 4, 0 + 4,
            1 + 4, 2 + 4, 6 + 4, 6 + 4, 5 + 4, 1 + 4,
            2 + 4, 3 + 4, 7 + 4, 7 + 4, 6 + 4, 2 + 4,
            3 + 4, 0 + 4, 4 + 4, 4 + 4, 7 + 4, 3 + 4,

            0 + 8, 1 + 8, 5 + 8, 5 + 8, 4 + 8, 0 + 8,
            1 + 8, 2 + 8, 6 + 8, 6 + 8, 5 + 8, 1 + 8,
            2 + 8, 3 + 8, 7 + 8, 7 + 8, 6 + 8, 2 + 8,
            3 + 8, 0 + 8, 4 + 8, 4 + 8, 7 + 8, 3 + 8,
        };
        // clang-format on
        static_assert(std::size(gMiterIndices) == kMiterIndexCnt);
        SKGPU_DEFINE_STATIC_UNIQUE_KEY(gMiterIndexBufferKey);
        return resourceProvider->findOrCreatePatternedIndexBuffer(
                gMiterIndices, kMiterIndexCnt, kNumMiterRectsInIndexBuffer, kMiterVertexCnt,
                gMiterIndexBufferKey);
    } else {
        /**
         * As in miter-stroke, index = a + b, and a is the current index, b is the shift
         * from the first index. The index layout:
         * outer AA line: 0~3, 4~7
         * outer edge:    8~11, 12~15
         * inner edge:    16~19
         * inner AA line: 20~23
         * Following comes a bevel-stroke rect and its indices:
         *
         *           4                                 7
         *            *********************************
         *          *   ______________________________  *
         *         *  / 12                          15 \  *
         *        *  /                                  \  *
         *     0 *  |8     16_____________________19  11 |  * 3
         *       *  |       |                    |       |  *
         *       *  |       |  ****************  |       |  *
         *       *  |       |  * 20        23 *  |       |  *
         *       *  |       |  *              *  |       |  *
         *       *  |       |  * 21        22 *  |       |  *
         *       *  |       |  ****************  |       |  *
         *       *  |       |____________________|       |  *
         *     1 *  |9    17                      18   10|  * 2
         *        *  \                                  /  *
         *         *  \13 __________________________14/  *
         *          *                                   *
         *           **********************************
         *          5                                  6
         */
        // clang-format off
        static const uint16_t gBevelIndices[] = {
            // Draw outer AA, from outer AA line to outer edge, shift is 0.
            0 + 0, 1 + 0,  9 + 0,  9 + 0,  8 + 0, 0 + 0,
            1 + 0, 5 + 0, 13 + 0, 13 + 0,  9 + 0, 1 + 0,
            5 + 0, 6 + 0, 14 + 0, 14 + 0, 13 + 0, 5 + 0,
            6 + 0, 2 + 0, 10 + 0, 10 + 0, 14 + 0, 6 + 0,
            2 + 0, 3 + 0, 11 + 0, 11 + 0, 10 + 0, 2 + 0,
            3 + 0, 7 + 0, 15 + 0, 15 + 0, 11 + 0, 3 + 0,
            7 + 0, 4 + 0, 12 + 0, 12 + 0, 15 + 0, 7 + 0,
            4 + 0, 0 + 0,  8 + 0,  8 + 0, 12 + 0, 4 + 0,

            // Draw the stroke, from outer edge to inner edge, shift is 8.
            0 + 8, 1 + 8, 9 + 8, 9 + 8, 8 + 8, 0 + 8,
            1 + 8, 5 + 8, 9 + 8,
            5 + 8, 6 + 8, 10 + 8, 10 + 8, 9 + 8, 5 + 8,
            6 + 8, 2 + 8, 10 + 8,
            2 + 8, 3 + 8, 11 + 8, 11 + 8, 10 + 8, 2 + 8,
            3 + 8, 7 + 8, 11 + 8,
            7 + 8, 4 + 8, 8 + 8, 8 + 8, 11 + 8, 7 + 8,
            4 + 8, 0 + 8, 8 + 8,

            // Draw the inner AA, from inner edge to inner AA line, shift is 16.
            0 + 16, 1 + 16, 5 + 16, 5 + 16, 4 + 16, 0 + 16,
            1 + 16, 2 + 16, 6 + 16, 6 + 16, 5 + 16, 1 + 16,
            2 + 16, 3 + 16, 7 + 16, 7 + 16, 6 + 16, 2 + 16,
            3 + 16, 0 + 16, 4 + 16, 4 + 16, 7 + 16, 3 + 16,
        };
        // clang-format on
        static_assert(std::size(gBevelIndices) == kBevelIndexCnt);

        SKGPU_DEFINE_STATIC_UNIQUE_KEY(gBevelIndexBufferKey);
        return resourceProvider->findOrCreatePatternedIndexBuffer(
                gBevelIndices, kBevelIndexCnt, kNumBevelRectsInIndexBuffer, kBevelVertexCnt,
                gBevelIndexBufferKey);
    }
}

GrOp::CombineResult AAStrokeRectOp::onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps)
{
    AAStrokeRectOp* that = t->cast<AAStrokeRectOp>();

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return CombineResult::kCannotCombine;
    }

    // TODO combine across miterstroke changes
    if (this->miterStroke() != that->miterStroke()) {
        return CombineResult::kCannotCombine;
    }

    // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
    // local coords then we won't be able to combine. TODO: Upload local coords as an attribute.
    if (fHelper.usesLocalCoords() &&
        !SkMatrixPriv::CheapEqual(this->viewMatrix(), that->viewMatrix()))
    {
        return CombineResult::kCannotCombine;
    }

    fRects.push_back_n(that->fRects.size(), that->fRects.begin());
    fWideColor |= that->fWideColor;
    return CombineResult::kMerged;
}

void AAStrokeRectOp::generateAAStrokeRectGeometry(VertexWriter& vertices,
                                                  const SkPMColor4f& color,
                                                  bool wideColor,
                                                  const SkRect& devOutside,
                                                  const SkRect& devOutsideAssist,
                                                  const SkRect& devInside,
                                                  bool miterStroke,
                                                  bool degenerate,
                                                  const SkVector& devHalfStrokeSize,
                                                  bool usesMSAASurface) const {
    // We create vertices for four nested rectangles. There are two ramps from 0 to full
    // coverage, one on the exterior of the stroke and the other on the interior.

    // The following code only works if either devStrokeSize's fX and fY are
    // equal (in which case innerCoverage is same for all sides of the rects) or
    // if devStrokeSize's fX and fY are both greater than 1.0 (in which case
    // innerCoverage will always be 1).
    SkASSERT(stroke_dev_half_size_supported(devHalfStrokeSize));

    auto inset_fan = [](const SkRect& r, SkScalar dx, SkScalar dy) {
        return VertexWriter::TriFanFromRect(r.makeInset(dx, dy));
    };

    bool tweakAlphaForCoverage = this->compatibleWithCoverageAsAlpha(usesMSAASurface);

    auto maybe_coverage = [tweakAlphaForCoverage](float coverage) {
        return VertexWriter::If(!tweakAlphaForCoverage, coverage);
    };

    // How much do we inset toward the inside of the strokes?
    float inset = std::min(0.5f, std::min(devHalfStrokeSize.fX, devHalfStrokeSize.fY));
    float innerCoverage = 1;
    if (inset < 0.5f) {
        // Stroke is subpixel, so reduce the coverage to simulate the narrower strokes.
        innerCoverage = 2 * inset / (inset + .5f);
    }

    // How much do we outset away from the outside of the strokes?
    // We always want to keep the AA picture frame one pixel wide.
    float outset = 1 - inset;
    float outerCoverage = 0;

    // How much do we outset away from the interior side of the stroke (toward the center)?
    float interiorOutset = outset;
    float interiorCoverage = outerCoverage;

    if (usesMSAASurface) {
        // Since we're using MSAA, extend our outsets to ensure any pixel with partial coverage has
        // a full sample mask.
        constexpr float msaaExtraBloat = SK_ScalarSqrt2 - .5f;
        outset += msaaExtraBloat;
        outerCoverage -= msaaExtraBloat;

        float insetExtraBloat =
                std::min(inset + msaaExtraBloat,
                         std::min(devHalfStrokeSize.fX, devHalfStrokeSize.fY)) - inset;
        inset += insetExtraBloat;
        innerCoverage += insetExtraBloat;

        float interiorExtraBloat =
                std::min(interiorOutset + msaaExtraBloat,
                         std::min(devInside.width(), devInside.height()) / 2) - interiorOutset;
        interiorOutset += interiorExtraBloat;
        interiorCoverage -= interiorExtraBloat;
    }

    VertexColor innerColor(tweakAlphaForCoverage ? color * innerCoverage : color, wideColor);
    VertexColor outerColor(tweakAlphaForCoverage ? SK_PMColor4fTRANSPARENT : color, wideColor);

    // Exterior outset rect (away from stroke).
    vertices.writeQuad(inset_fan(devOutside, -outset, -outset),
                       outerColor,
                       maybe_coverage(outerCoverage));

    if (!miterStroke) {
        // Second exterior outset.
        vertices.writeQuad(inset_fan(devOutsideAssist, -outset, -outset),
                           outerColor,
                           maybe_coverage(outerCoverage));
    }

    // Exterior inset rect (toward stroke).
    vertices.writeQuad(inset_fan(devOutside, inset, inset),
                       innerColor,
                       maybe_coverage(innerCoverage));

    if (!miterStroke) {
        // Second exterior inset.
        vertices.writeQuad(inset_fan(devOutsideAssist, inset, inset),
                           innerColor,
                           maybe_coverage(innerCoverage));
    }

    if (!degenerate) {
        // Interior inset rect (toward stroke).
        vertices.writeQuad(inset_fan(devInside, -inset, -inset),
                           innerColor,
                           maybe_coverage(innerCoverage));

        // Interior outset rect (away from stroke, toward center of rect).
        SkRect interiorAABoundary = devInside.makeInset(interiorOutset, interiorOutset);
        float coverageBackset = 0;  // Adds back coverage when the interior AA edges cross.
        if (interiorAABoundary.fLeft > interiorAABoundary.fRight) {
            coverageBackset =
                    (interiorAABoundary.fLeft - interiorAABoundary.fRight) / (interiorOutset * 2);
            interiorAABoundary.fLeft = interiorAABoundary.fRight = interiorAABoundary.centerX();
        }
        if (interiorAABoundary.fTop > interiorAABoundary.fBottom) {
            coverageBackset = std::max(
                    (interiorAABoundary.fTop - interiorAABoundary.fBottom) / (interiorOutset * 2),
                    coverageBackset);
            interiorAABoundary.fTop = interiorAABoundary.fBottom = interiorAABoundary.centerY();
        }
        if (coverageBackset > 0) {
            // The interior edges crossed. Lerp back toward innerCoverage, which is what this op
            // will draw in the degenerate case. This gives a smooth transition into the degenerate
            // case.
            interiorCoverage += interiorCoverage * (1 - coverageBackset) +
                                innerCoverage * coverageBackset;
        }
        VertexColor interiorColor(tweakAlphaForCoverage ? color * interiorCoverage : color,
                                  wideColor);
        vertices.writeQuad(VertexWriter::TriFanFromRect(interiorAABoundary),
                           interiorColor,
                           maybe_coverage(interiorCoverage));
    } else {
        // When the interior rect has become degenerate we smoosh to a single point
        SkASSERT(devInside.fLeft == devInside.fRight && devInside.fTop == devInside.fBottom);

        vertices.writeQuad(VertexWriter::TriFanFromRect(devInside),
                           innerColor,
                           maybe_coverage(innerCoverage));

        // ... unless we are degenerate, in which case we must apply the scaled coverage
        vertices.writeQuad(VertexWriter::TriFanFromRect(devInside),
                           innerColor,
                           maybe_coverage(innerCoverage));
    }
}

}  // anonymous namespace

GrOp::Owner Make(GrRecordingContext* context,
                 GrPaint&& paint,
                 GrAAType aaType,
                 const SkMatrix& viewMatrix,
                 const SkRect& rect,
                 const SkStrokeRec& stroke) {
    SkASSERT(!context->priv().caps()->reducedShaderMode());
    if (aaType == GrAAType::kCoverage) {
        return AAStrokeRectOp::Make(context, std::move(paint), viewMatrix, rect, stroke);
    } else {
        return NonAAStrokeRectOp::Make(context, std::move(paint), viewMatrix, rect, stroke, aaType);
    }
}

GrOp::Owner MakeNested(GrRecordingContext* context,
                       GrPaint&& paint,
                       const SkMatrix& viewMatrix,
                       const SkRect rects[2]) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(!rects[0].isEmpty() && !rects[1].isEmpty());

    SkRect devOutside = viewMatrix.mapRect(rects[0]);
    SkRect devInside = viewMatrix.mapRect(rects[1]);
    float dx = devOutside.fRight - devInside.fRight;
    float dy = devOutside.fBottom - devInside.fBottom;

    // Clips our draw rects 1 full pixel outside the viewport. This avoids interpolation precision
    // issues with very large coordinates.
    const float m = context->priv().caps()->maxRenderTargetSize();
    const SkRect visibilityBounds = SkRect::MakeWH(m, m).makeOutset(1, 1);

    if (!devOutside.intersect(visibilityBounds.makeOutset(dx, dy))) {
        return nullptr;
    }

    if (devInside.isEmpty() || !devInside.intersect(visibilityBounds)) {
        if (devOutside.isEmpty()) {
            return nullptr;
        }
        DrawQuad quad{GrQuad::MakeFromRect(rects[0], viewMatrix), GrQuad(rects[0]),
                      GrQuadAAFlags::kAll};
        return ganesh::FillRectOp::Make(context, std::move(paint), GrAAType::kCoverage, &quad);
    }

    return AAStrokeRectOp::Make(context, std::move(paint), viewMatrix, devOutside,
                                devInside, SkVector{dx, dy} * .5f);
}

} // namespace skgpu::ganesh::StrokeRectOp

#if defined(GPU_TEST_UTILS)

GR_DRAW_OP_TEST_DEFINE(NonAAStrokeRectOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    SkRect rect = GrTest::TestRect(random);
    SkScalar strokeWidth = random->nextBool() ? 0.0f : 2.0f;
    SkPaint strokePaint;
    strokePaint.setStrokeWidth(strokeWidth);
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeJoin(SkPaint::kMiter_Join);
    SkStrokeRec strokeRec(strokePaint);
    GrAAType aaType = GrAAType::kNone;
    if (numSamples > 1) {
        aaType = random->nextBool() ? GrAAType::kMSAA : GrAAType::kNone;
    }
    return skgpu::ganesh::StrokeRectOp::NonAAStrokeRectOp::Make(context, std::move(paint),
                                                                viewMatrix, rect, strokeRec,
                                                                aaType);
}

GR_DRAW_OP_TEST_DEFINE(AAStrokeRectOp) {
    bool miterStroke = random->nextBool();

    // Create either a empty rect or a non-empty rect.
    SkRect rect =
            random->nextBool() ? SkRect::MakeXYWH(10, 10, 50, 40) : SkRect::MakeXYWH(6, 7, 0, 0);
    SkScalar minDim = std::min(rect.width(), rect.height());
    SkScalar strokeWidth = random->nextUScalar1() * minDim;

    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    rec.setStrokeStyle(strokeWidth);
    rec.setStrokeParams(SkPaint::kButt_Cap,
                        miterStroke ? SkPaint::kMiter_Join : SkPaint::kBevel_Join, 1.f);
    SkMatrix matrix = GrTest::TestMatrixRectStaysRect(random);
    return skgpu::ganesh::StrokeRectOp::AAStrokeRectOp::Make(context, std::move(paint), matrix,
                                                             rect, rec);
}

#endif
