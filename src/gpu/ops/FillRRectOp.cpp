/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/FillRRectOp.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

namespace skgpu::v1::FillRRectOp {

namespace {

class FillRRectOpImpl final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext*,
                            SkArenaAlloc*,
                            GrPaint&&,
                            const SkMatrix& viewMatrix,
                            const SkRRect&,
                            const SkRect& localRect,
                            GrAA);

    const char* name() const override { return "FillRRectOp"; }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    ClipResult clipToShape(skgpu::v1::SurfaceDrawContext*,
                           SkClipOp,
                           const SkMatrix& clipMatrix,
                           const GrShape&,
                           GrAA) override;

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override;
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) override;

    void visitProxies(const GrVisitProxyFunc& func) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        } else {
            fHelper.visitProxies(func);
        }
    }

    void onPrepareDraws(GrMeshDrawTarget*) override;

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

private:
    friend class ::GrSimpleMeshDrawOpHelper; // for access to ctor
    friend class ::GrOp;         // for access to ctor

    enum class ProcessorFlags {
        kNone             = 0,
        kUseHWDerivatives = 1 << 0,
        kHasLocalCoords   = 1 << 1,
        kWideColor        = 1 << 2,
        kMSAAEnabled      = 1 << 3,
        kFakeNonAA        = 1 << 4,
    };
    constexpr static int kNumProcessorFlags = 5;

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(ProcessorFlags);

    class Processor;

    FillRRectOpImpl(GrProcessorSet*,
                    const SkPMColor4f& paintColor,
                    SkArenaAlloc*,
                    const SkMatrix& viewMatrix,
                    const SkRRect&,
                    const SkRect& localRect,
                    ProcessorFlags);

    GrProgramInfo* programInfo() override { return fProgramInfo; }

    // Create a GrProgramInfo object in the provided arena
    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&&,
                             const GrDstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override;

    Helper         fHelper;
    ProcessorFlags fProcessorFlags;

    struct Instance {
        Instance(const SkMatrix& viewMatrix, const SkRRect& rrect, const SkRect& localRect,
                 const SkPMColor4f& color)
                : fViewMatrix(viewMatrix), fRRect(rrect), fLocalRect(localRect), fColor(color) {}
        SkMatrix fViewMatrix;
        SkRRect fRRect;
        SkRect fLocalRect;
        SkPMColor4f fColor;
        Instance* fNext = nullptr;
    };

    Instance* fHeadInstance;
    Instance** fTailInstance;
    int fInstanceCount = 1;

    sk_sp<const GrBuffer> fInstanceBuffer;
    sk_sp<const GrBuffer> fVertexBuffer;
    sk_sp<const GrBuffer> fIndexBuffer;
    int fBaseInstance = 0;

    // If this op is prePrepared the created programInfo will be stored here for use in
    // onExecute. In the prePrepared case it will have been stored in the record-time arena.
    GrProgramInfo* fProgramInfo = nullptr;
};

GR_MAKE_BITFIELD_CLASS_OPS(FillRRectOpImpl::ProcessorFlags)

// Hardware derivatives are not always accurate enough for highly elliptical corners. This method
// checks to make sure the corners will still all look good if we use HW derivatives.
bool can_use_hw_derivatives_with_coverage(const GrShaderCaps&,
                                          const SkMatrix&,
                                          const SkRRect&);

GrOp::Owner FillRRectOpImpl::Make(GrRecordingContext* ctx,
                                  SkArenaAlloc* arena,
                                  GrPaint&& paint,
                                  const SkMatrix& viewMatrix,
                                  const SkRRect& rrect,
                                  const SkRect& localRect,
                                  GrAA aa) {
    const GrCaps* caps = ctx->priv().caps();

    if (!caps->drawInstancedSupport()) {
        return nullptr;
    }

    // We transform into a normalized -1..+1 space to draw the round rect. If the boundaries are too
    // large, the math can overflow. The caller can fall back on path rendering if this is the case.
    if (std::max(rrect.height(), rrect.width()) >= 1e6f) {
        return nullptr;
    }

    ProcessorFlags flags = ProcessorFlags::kNone;
    // TODO: Support perspective in a follow-on CL. This shouldn't be difficult, since we already
    // use HW derivatives. The only trick will be adjusting the AA outset to account for
    // perspective. (i.e., outset = 0.5 * z.)
    if (viewMatrix.hasPerspective()) {
        return nullptr;
    }
    if (can_use_hw_derivatives_with_coverage(*caps->shaderCaps(), viewMatrix, rrect)) {
        // HW derivatives (more specifically, fwidth()) are consistently faster on all platforms in
        // coverage mode. We use them as long as the approximation will be accurate enough.
        flags |= ProcessorFlags::kUseHWDerivatives;
    }
    if (aa == GrAA::kNo) {
        flags |= ProcessorFlags::kFakeNonAA;
    }

    return Helper::FactoryHelper<FillRRectOpImpl>(ctx, std::move(paint), arena, viewMatrix, rrect,
                                                  localRect, flags);
}

FillRRectOpImpl::FillRRectOpImpl(GrProcessorSet* processorSet,
                                 const SkPMColor4f& paintColor,
                                 SkArenaAlloc* arena,
                                 const SkMatrix& viewMatrix,
                                 const SkRRect& rrect,
                                 const SkRect& localRect,
                                 ProcessorFlags processorFlags)
        : GrMeshDrawOp(ClassID())
        , fHelper(processorSet,
                  (processorFlags & ProcessorFlags::kFakeNonAA)
                          ? GrAAType::kNone
                          : GrAAType::kCoverage)  // Use analytic AA even if the RT is MSAA.
        , fProcessorFlags(processorFlags & ~(ProcessorFlags::kHasLocalCoords |
                                             ProcessorFlags::kWideColor |
                                             ProcessorFlags::kMSAAEnabled))
        , fHeadInstance(arena->make<Instance>(viewMatrix, rrect, localRect, paintColor))
        , fTailInstance(&fHeadInstance->fNext) {
    // FillRRectOp::Make fails if there is perspective.
    SkASSERT(!viewMatrix.hasPerspective());
    this->setBounds(viewMatrix.mapRect(rrect.getBounds()),
                    GrOp::HasAABloat(!(processorFlags & ProcessorFlags::kFakeNonAA)),
                    GrOp::IsHairline::kNo);
}

GrDrawOp::ClipResult FillRRectOpImpl::clipToShape(skgpu::v1::SurfaceDrawContext* sdc,
                                                  SkClipOp clipOp,
                                                  const SkMatrix& clipMatrix,
                                                  const GrShape& shape,
                                                  GrAA aa) {
    SkASSERT(fInstanceCount == 1);  // This needs to be called before combining.
    SkASSERT(fHeadInstance->fNext == nullptr);

    if ((shape.isRect() || shape.isRRect()) &&
        clipOp == SkClipOp::kIntersect &&
        (aa == GrAA::kNo) == (fProcessorFlags & ProcessorFlags::kFakeNonAA)) {
        // The clip shape is a round rect. Attempt to map it to a round rect in "viewMatrix" space.
        SkRRect clipRRect;
        if (clipMatrix == fHeadInstance->fViewMatrix) {
            if (shape.isRect()) {
                clipRRect.setRect(shape.rect());
            } else {
                clipRRect = shape.rrect();
            }
        } else {
            // Find a matrix that maps from "clipMatrix" space to "viewMatrix" space.
            SkASSERT(!fHeadInstance->fViewMatrix.hasPerspective());
            if (clipMatrix.hasPerspective()) {
                return ClipResult::kFail;
            }
            SkMatrix clipToView;
            if (!fHeadInstance->fViewMatrix.invert(&clipToView)) {
                return ClipResult::kClippedOut;
            }
            clipToView.preConcat(clipMatrix);
            SkASSERT(!clipToView.hasPerspective());
            if (!SkScalarNearlyZero(clipToView.getSkewX()) ||
                !SkScalarNearlyZero(clipToView.getSkewY())) {
                // A rect in "clipMatrix" space is not a rect in "viewMatrix" space.
                return ClipResult::kFail;
            }
            clipToView.setSkewX(0);
            clipToView.setSkewY(0);
            SkASSERT(clipToView.rectStaysRect());

            if (shape.isRect()) {
                clipRRect.setRect(clipToView.mapRect(shape.rect()));
            } else {
                if (!shape.rrect().transform(clipToView, &clipRRect)) {
                    // Transforming the rrect failed. This shouldn't generally happen except in
                    // cases of fp32 overflow.
                    return ClipResult::kFail;
                }
            }
        }

        // Intersect our round rect with the clip shape.
        SkRRect isectRRect;
        if (fHeadInstance->fRRect.isRect() && clipRRect.isRect()) {
            SkRect isectRect;
            if (!isectRect.intersect(fHeadInstance->fRRect.rect(), clipRRect.rect())) {
                return ClipResult::kClippedOut;
            }
            isectRRect.setRect(isectRect);
        } else {
            isectRRect = SkRRectPriv::ConservativeIntersect(fHeadInstance->fRRect, clipRRect);
            if (isectRRect.isEmpty()) {
                // The round rects did not intersect at all or the intersection was too complicated
                // to compute quickly.
                return ClipResult::kFail;
            }
        }

        // Don't apply the clip geometrically if it becomes subpixel, since then the hairline
        // rendering may outset beyond the original clip.
        SkRect devISectBounds = fHeadInstance->fViewMatrix.mapRect(isectRRect.rect());
        if (devISectBounds.width() < 1.f || devISectBounds.height() < 1.f) {
            return ClipResult::kFail;
        }

        // Update the local rect.
        auto rect = skvx::bit_pun<grvx::float4>(fHeadInstance->fRRect.rect());
        auto local = skvx::bit_pun<grvx::float4>(fHeadInstance->fLocalRect);
        auto isect = skvx::bit_pun<grvx::float4>(isectRRect.rect());
        auto rectToLocalSize = (local - skvx::shuffle<2,3,0,1>(local)) /
                               (rect - skvx::shuffle<2,3,0,1>(rect));
        fHeadInstance->fLocalRect = skvx::bit_pun<SkRect>((isect - rect) * rectToLocalSize + local);

        // Update the round rect.
        fHeadInstance->fRRect = isectRRect;
        return ClipResult::kClippedGeometrically;
    }

    return ClipResult::kFail;
}

GrProcessorSet::Analysis FillRRectOpImpl::finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                                   GrClampType clampType) {
    SkASSERT(fInstanceCount == 1);
    SkASSERT(fHeadInstance->fNext == nullptr);

    bool isWideColor;
    auto analysis = fHelper.finalizeProcessors(caps, clip, clampType,
                                               GrProcessorAnalysisCoverage::kSingleChannel,
                                               &fHeadInstance->fColor, &isWideColor);
    if (isWideColor) {
        fProcessorFlags |= ProcessorFlags::kWideColor;
    }
    if (analysis.usesLocalCoords()) {
        fProcessorFlags |= ProcessorFlags::kHasLocalCoords;
    }
    return analysis;
}

GrOp::CombineResult FillRRectOpImpl::onCombineIfPossible(GrOp* op,
                                                         SkArenaAlloc*,
                                                         const GrCaps& caps) {
    auto that = op->cast<FillRRectOpImpl>();
    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds()) ||
        fProcessorFlags != that->fProcessorFlags) {
        return CombineResult::kCannotCombine;
    }

    *fTailInstance = that->fHeadInstance;
    fTailInstance = that->fTailInstance;
    fInstanceCount += that->fInstanceCount;
    return CombineResult::kMerged;
}

class FillRRectOpImpl::Processor final : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, GrAAType aaType, ProcessorFlags flags) {
        return arena->make([&](void* ptr) {
            return new (ptr) Processor(aaType, flags);
        });
    }

    const char* name() const override { return "FillRRectOp::Processor"; }

    void addToKey(const GrShaderCaps& caps, KeyBuilder* b) const override {
        b->addBits(kNumProcessorFlags, (uint32_t)fFlags,  "flags");
    }

    std::unique_ptr<ProgramImpl> makeProgramImpl(const GrShaderCaps&) const override;

private:
    class Impl;

    Processor(GrAAType aaType, ProcessorFlags flags)
            : GrGeometryProcessor(kGrFillRRectOp_Processor_ClassID)
            , fFlags(flags) {
        this->setVertexAttributesWithImplicitOffsets(kVertexAttribs,
                                                     SK_ARRAY_COUNT(kVertexAttribs));

        fInstanceAttribs.emplace_back("skew", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fInstanceAttribs.emplace_back("translate", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        fInstanceAttribs.emplace_back("radii_x", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fInstanceAttribs.emplace_back("radii_y", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fColorAttrib = &fInstanceAttribs.push_back(
                MakeColorAttribute("color", (fFlags & ProcessorFlags::kWideColor)));
        if (fFlags & ProcessorFlags::kHasLocalCoords) {
            fInstanceAttribs.emplace_back(
                    "local_rect", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        }
        SkASSERT(fInstanceAttribs.count() <= kMaxInstanceAttribs);
        this->setInstanceAttributesWithImplicitOffsets(fInstanceAttribs.begin(),
                                                       fInstanceAttribs.count());
    }

    inline static constexpr Attribute kVertexAttribs[] = {
            {"radii_selector", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"corner_and_radius_outsets", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            // Coverage only.
            {"aa_bloat_and_coverage", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};

    const ProcessorFlags fFlags;

    constexpr static int kMaxInstanceAttribs = 6;
    SkSTArray<kMaxInstanceAttribs, Attribute> fInstanceAttribs;
    const Attribute* fColorAttrib;
};

// Our coverage geometry consists of an inset octagon with solid coverage, surrounded by linear
// coverage ramps on the horizontal and vertical edges, and "arc coverage" pieces on the diagonal
// edges. The Vertex struct tells the shader where to place its vertex within a normalized
// ([l, t, r, b] = [-1, -1, +1, +1]) space, and how to calculate coverage. See onEmitCode.
struct CoverageVertex {
    std::array<float, 4> fRadiiSelector;
    std::array<float, 2> fCorner;
    std::array<float, 2> fRadiusOutset;
    std::array<float, 2> fAABloatDirection;
    float fCoverage;
    float fIsLinearCoverage;
};

// This is the offset (when multiplied by radii) from the corners of a bounding box to the vertices
// of its inscribed octagon. We draw the outside portion of arcs with quarter-octagons rather than
// rectangles.
static constexpr float kOctoOffset = 1/(1 + SK_ScalarRoot2Over2);

static constexpr CoverageVertex kVertexData[] = {
        // Left inset edge.
        {{{0,0,0,1}},  {{-1,+1}},  {{0,-1}},  {{+1,0}},  1,  1},
        {{{1,0,0,0}},  {{-1,-1}},  {{0,+1}},  {{+1,0}},  1,  1},

        // Top inset edge.
        {{{1,0,0,0}},  {{-1,-1}},  {{+1,0}},  {{0,+1}},  1,  1},
        {{{0,1,0,0}},  {{+1,-1}},  {{-1,0}},  {{0,+1}},  1,  1},

        // Right inset edge.
        {{{0,1,0,0}},  {{+1,-1}},  {{0,+1}},  {{-1,0}},  1,  1},
        {{{0,0,1,0}},  {{+1,+1}},  {{0,-1}},  {{-1,0}},  1,  1},

        // Bottom inset edge.
        {{{0,0,1,0}},  {{+1,+1}},  {{-1,0}},  {{0,-1}},  1,  1},
        {{{0,0,0,1}},  {{-1,+1}},  {{+1,0}},  {{0,-1}},  1,  1},


        // Left outset edge.
        {{{0,0,0,1}},  {{-1,+1}},  {{0,-1}},  {{-1,0}},  0,  1},
        {{{1,0,0,0}},  {{-1,-1}},  {{0,+1}},  {{-1,0}},  0,  1},

        // Top outset edge.
        {{{1,0,0,0}},  {{-1,-1}},  {{+1,0}},  {{0,-1}},  0,  1},
        {{{0,1,0,0}},  {{+1,-1}},  {{-1,0}},  {{0,-1}},  0,  1},

        // Right outset edge.
        {{{0,1,0,0}},  {{+1,-1}},  {{0,+1}},  {{+1,0}},  0,  1},
        {{{0,0,1,0}},  {{+1,+1}},  {{0,-1}},  {{+1,0}},  0,  1},

        // Bottom outset edge.
        {{{0,0,1,0}},  {{+1,+1}},  {{-1,0}},  {{0,+1}},  0,  1},
        {{{0,0,0,1}},  {{-1,+1}},  {{+1,0}},  {{0,+1}},  0,  1},


        // Top-left corner.
        {{{1,0,0,0}},  {{-1,-1}},  {{ 0,+1}},  {{-1, 0}},  0,  0},
        {{{1,0,0,0}},  {{-1,-1}},  {{ 0,+1}},  {{+1, 0}},  1,  0},
        {{{1,0,0,0}},  {{-1,-1}},  {{+1, 0}},  {{ 0,+1}},  1,  0},
        {{{1,0,0,0}},  {{-1,-1}},  {{+1, 0}},  {{ 0,-1}},  0,  0},
        {{{1,0,0,0}},  {{-1,-1}},  {{+kOctoOffset,0}},  {{-1,-1}},  0,  0},
        {{{1,0,0,0}},  {{-1,-1}},  {{0,+kOctoOffset}},  {{-1,-1}},  0,  0},

        // Top-right corner.
        {{{0,1,0,0}},  {{+1,-1}},  {{-1, 0}},  {{ 0,-1}},  0,  0},
        {{{0,1,0,0}},  {{+1,-1}},  {{-1, 0}},  {{ 0,+1}},  1,  0},
        {{{0,1,0,0}},  {{+1,-1}},  {{ 0,+1}},  {{-1, 0}},  1,  0},
        {{{0,1,0,0}},  {{+1,-1}},  {{ 0,+1}},  {{+1, 0}},  0,  0},
        {{{0,1,0,0}},  {{+1,-1}},  {{0,+kOctoOffset}},  {{+1,-1}},  0,  0},
        {{{0,1,0,0}},  {{+1,-1}},  {{-kOctoOffset,0}},  {{+1,-1}},  0,  0},

        // Bottom-right corner.
        {{{0,0,1,0}},  {{+1,+1}},  {{ 0,-1}},  {{+1, 0}},  0,  0},
        {{{0,0,1,0}},  {{+1,+1}},  {{ 0,-1}},  {{-1, 0}},  1,  0},
        {{{0,0,1,0}},  {{+1,+1}},  {{-1, 0}},  {{ 0,-1}},  1,  0},
        {{{0,0,1,0}},  {{+1,+1}},  {{-1, 0}},  {{ 0,+1}},  0,  0},
        {{{0,0,1,0}},  {{+1,+1}},  {{-kOctoOffset,0}},  {{+1,+1}},  0,  0},
        {{{0,0,1,0}},  {{+1,+1}},  {{0,-kOctoOffset}},  {{+1,+1}},  0,  0},

        // Bottom-left corner.
        {{{0,0,0,1}},  {{-1,+1}},  {{+1, 0}},  {{ 0,+1}},  0,  0},
        {{{0,0,0,1}},  {{-1,+1}},  {{+1, 0}},  {{ 0,-1}},  1,  0},
        {{{0,0,0,1}},  {{-1,+1}},  {{ 0,-1}},  {{+1, 0}},  1,  0},
        {{{0,0,0,1}},  {{-1,+1}},  {{ 0,-1}},  {{-1, 0}},  0,  0},
        {{{0,0,0,1}},  {{-1,+1}},  {{0,-kOctoOffset}},  {{-1,+1}},  0,  0},
        {{{0,0,0,1}},  {{-1,+1}},  {{+kOctoOffset,0}},  {{-1,+1}},  0,  0}};

SKGPU_DECLARE_STATIC_UNIQUE_KEY(gVertexBufferKey);

static constexpr uint16_t kIndexData[] = {
        // Inset octagon (solid coverage).
        0, 1, 7,
        1, 2, 7,
        7, 2, 6,
        2, 3, 6,
        6, 3, 5,
        3, 4, 5,

        // AA borders (linear coverage).
        0, 1, 8, 1, 9, 8,
        2, 3, 10, 3, 11, 10,
        4, 5, 12, 5, 13, 12,
        6, 7, 14, 7, 15, 14,

        // Top-left arc.
        16, 17, 21,
        17, 21, 18,
        21, 18, 20,
        18, 20, 19,

        // Top-right arc.
        22, 23, 27,
        23, 27, 24,
        27, 24, 26,
        24, 26, 25,

        // Bottom-right arc.
        28, 29, 33,
        29, 33, 30,
        33, 30, 32,
        30, 32, 31,

        // Bottom-left arc.
        34, 35, 39,
        35, 39, 36,
        39, 36, 38,
        36, 38, 37};

SKGPU_DECLARE_STATIC_UNIQUE_KEY(gIndexBufferKey);

void FillRRectOpImpl::onPrepareDraws(GrMeshDrawTarget* target) {
    if (!fProgramInfo) {
        this->createProgramInfo(target);
    }

    size_t instanceStride = fProgramInfo->geomProc().instanceStride();

    if (VertexWriter instanceWrter = target->makeVertexSpace(instanceStride, fInstanceCount,
                                                             &fInstanceBuffer, &fBaseInstance)) {
        SkDEBUGCODE(auto end = instanceWrter.makeOffset(instanceStride * fInstanceCount));
        for (Instance* i = fHeadInstance; i; i = i->fNext) {
            auto [l, t, r, b] = i->fRRect.rect();

            // Produce a matrix that draws the round rect from normalized [-1, -1, +1, +1] space.
            SkMatrix m;
            // Unmap the normalized rect [-1, -1, +1, +1] back to [l, t, r, b].
            m.setScaleTranslate((r - l)/2, (b - t)/2, (l + r)/2, (t + b)/2);
            // Map to device space.
            m.postConcat(i->fViewMatrix);

            // Convert the radii to [-1, -1, +1, +1] space and write their attribs.
            grvx::float4 radiiX, radiiY;
            skvx::strided_load2(&SkRRectPriv::GetRadiiArray(i->fRRect)->fX, radiiX, radiiY);
            radiiX *= 2 / (r - l);
            radiiY *= 2 / (b - t);

            instanceWrter << m.getScaleX() << m.getSkewX() << m.getSkewY() << m.getScaleY()
                          << m.getTranslateX() << m.getTranslateY()
                          << radiiX << radiiY
                          << VertexColor(i->fColor, fProcessorFlags & ProcessorFlags::kWideColor)
                          << VertexWriter::If(fProcessorFlags & ProcessorFlags::kHasLocalCoords,
                                              i->fLocalRect);
        }
        SkASSERT(instanceWrter == end);
    }

    SKGPU_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);

    fIndexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                                      sizeof(kIndexData),
                                                                      kIndexData, gIndexBufferKey);

    SKGPU_DEFINE_STATIC_UNIQUE_KEY(gVertexBufferKey);

    fVertexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(GrGpuBufferType::kVertex,
                                                                      sizeof(kVertexData),
                                                                      kVertexData,
                                                                      gVertexBufferKey);
}

class FillRRectOpImpl::Processor::Impl : public ProgramImpl {
public:
    void setData(const GrGLSLProgramDataManager&,
                 const GrShaderCaps&,
                 const GrGeometryProcessor&) override {}

private:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        GrGLSLVertexBuilder* v = args.fVertBuilder;
        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

        const auto& proc = args.fGeomProc.cast<Processor>();
        bool useHWDerivatives = (proc.fFlags & ProcessorFlags::kUseHWDerivatives);

        SkASSERT(proc.vertexStride() == sizeof(CoverageVertex));

        GrGLSLVaryingHandler* varyings = args.fVaryingHandler;
        varyings->emitAttributes(proc);
        f->codeAppendf("half4 %s;", args.fOutputColor);
        varyings->addPassThroughAttribute(proc.fColorAttrib->asShaderVar(),
                                          args.fOutputColor,
                                          GrGLSLVaryingHandler::Interpolation::kCanBeFlat);

        // Emit the vertex shader.
        // When MSAA is enabled, we need to make sure every sample gets lit up on pixels that have
        // fractional coverage. We do this by making the ramp wider.
        v->codeAppendf("float aa_bloat_multiplier = %i;",
                       (proc.fFlags & ProcessorFlags::kMSAAEnabled)
                               ? 2    // Outset an entire pixel (2 radii).
                       : (!(proc.fFlags & ProcessorFlags::kFakeNonAA))
                               ? 1    // Outset one half pixel (1 radius).
                               : 0);  // No AA bloat.

        // Unpack vertex attribs.
        v->codeAppend("float2 corner = corner_and_radius_outsets.xy;");
        v->codeAppend("float2 radius_outset = corner_and_radius_outsets.zw;");
        v->codeAppend("float2 aa_bloat_direction = aa_bloat_and_coverage.xy;");
        v->codeAppend("float is_linear_coverage = aa_bloat_and_coverage.w;");

        // Find the amount to bloat each edge for AA (in source space).
        v->codeAppend("float2 pixellength = inversesqrt("
                              "float2(dot(skew.xz, skew.xz), dot(skew.yw, skew.yw)));");
        v->codeAppend("float4 normalized_axis_dirs = skew * pixellength.xyxy;");
        v->codeAppend("float2 axiswidths = (abs(normalized_axis_dirs.xy) + "
                                           "abs(normalized_axis_dirs.zw));");
        v->codeAppend("float2 aa_bloatradius = axiswidths * pixellength * .5;");

        // Identify our radii.
        v->codeAppend("float4 radii_and_neighbors = radii_selector"
                              "* float4x4(radii_x, radii_y, radii_x.yxwz, radii_y.wzyx);");
        v->codeAppend("float2 radii = radii_and_neighbors.xy;");
        v->codeAppend("float2 neighbor_radii = radii_and_neighbors.zw;");

        v->codeAppend("float coverage_multiplier = 1;");
        v->codeAppend("if (any(greaterThan(aa_bloatradius, float2(1)))) {");
                          // The rrect is more narrow than a half-pixel AA coverage ramp. We can't
                          // draw as-is or else opposite AA borders will overlap. Instead, fudge the
                          // size up to the width of a coverage ramp, and then reduce total coverage
                          // to make the rect appear more thin.
        v->codeAppend(    "corner = max(abs(corner), aa_bloatradius) * sign(corner);");
        v->codeAppend(    "coverage_multiplier = 1 / (max(aa_bloatradius.x, 1) * "
                                                     "max(aa_bloatradius.y, 1));");
                          // Set radii to zero to ensure we take the "linear coverage" codepath.
                          // (The "coverage" variable only has effect in the linear codepath.)
        v->codeAppend(    "radii = float2(0);");
        v->codeAppend("}");

        // Unpack coverage.
        v->codeAppend("float coverage = aa_bloat_and_coverage.z;");
        if (proc.fFlags & ProcessorFlags::kMSAAEnabled) {
            // MSAA has a wider ramp that goes from -.5 to 1.5 instead of 0 to 1.
            v->codeAppendf("coverage = (coverage - .5) * aa_bloat_multiplier + .5;");
        }

        v->codeAppend("if (any(lessThan(radii, aa_bloatradius * 1.5))) {");
                          // The radii are very small. Demote this arc to a sharp 90 degree corner.
        v->codeAppend(    "radii = float2(0);");
                          // Convert to a standard picture frame for an AA rect instead of the round
                          // rect geometry.
        v->codeAppend(    "aa_bloat_direction = sign(corner);");
        v->codeAppend(    "if (coverage > .5) {");  // Are we an inset edge?
        v->codeAppend(        "aa_bloat_direction = -aa_bloat_direction;");
        v->codeAppend(    "}");
        v->codeAppend(    "is_linear_coverage = 1;");
        v->codeAppend("} else {");
                          // Don't let radii get smaller than a coverage ramp plus an extra half
                          // pixel for MSAA. Always use the same amount so we don't pop when
                          // switching between MSAA and coverage.
        v->codeAppend(    "radii = clamp(radii, pixellength * 1.5, 2 - pixellength * 1.5);");
        v->codeAppend(    "neighbor_radii = clamp(neighbor_radii, pixellength * 1.5, "
                                                 "2 - pixellength * 1.5);");
                          // Don't let neighboring radii get closer together than 1/16 pixel.
        v->codeAppend(    "float2 spacing = 2 - radii - neighbor_radii;");
        v->codeAppend(    "float2 extra_pad = max(pixellength * .0625 - spacing, float2(0));");
        v->codeAppend(    "radii -= extra_pad * .5;");
        v->codeAppend("}");

        // Find our vertex position, adjusted for radii and bloated for AA. Our rect is drawn in
        // normalized [-1,-1,+1,+1] space.
        v->codeAppend("float2 aa_outset = "
                              "aa_bloat_direction * aa_bloatradius * aa_bloat_multiplier;");
        v->codeAppend("float2 vertexpos = corner + radius_outset * radii + aa_outset;");

        v->codeAppend("if (coverage > .5) {");  // Are we an inset edge?
                          // Don't allow the aa insets to overlap. i.e., Don't let them inset past
                          // the center (x=y=0). Since we don't allow the rect to become thinner
                          // than 1px, this should only happen when using MSAA, where we inset by an
                          // entire pixel instead of half.
        v->codeAppend(    "if (aa_bloat_direction.x != 0 && vertexpos.x * corner.x < 0) {");
        v->codeAppend(        "float backset = abs(vertexpos.x);");
        v->codeAppend(        "vertexpos.x = 0;");
        v->codeAppend(        "vertexpos.y += "
                                      "backset * sign(corner.y) * pixellength.y/pixellength.x;");
        v->codeAppend(        "coverage = (coverage - .5) * abs(corner.x) / "
                                      "(abs(corner.x) + backset) + .5;");
        v->codeAppend(    "}");
        v->codeAppend(    "if (aa_bloat_direction.y != 0 && vertexpos.y * corner.y < 0) {");
        v->codeAppend(        "float backset = abs(vertexpos.y);");
        v->codeAppend(        "vertexpos.y = 0;");
        v->codeAppend(        "vertexpos.x += "
                                      "backset * sign(corner.x) * pixellength.x/pixellength.y;");
        v->codeAppend(        "coverage = (coverage - .5) * abs(corner.y) / "
                                      "(abs(corner.y) + backset) + .5;");
        v->codeAppend(    "}");
        v->codeAppend("}");

        // Write positions
        GrShaderVar localCoord("", kFloat2_GrSLType);
        if (proc.fFlags & ProcessorFlags::kHasLocalCoords) {
            v->codeAppend("float2 localcoord = (local_rect.xy * (1 - vertexpos) + "
                                               "local_rect.zw * (1 + vertexpos)) * .5;");
            gpArgs->fLocalCoordVar.set(kFloat2_GrSLType, "localcoord");
        }

        // Transform to device space.
        v->codeAppend("float2x2 skewmatrix = float2x2(skew.xy, skew.zw);");
        v->codeAppend("float2 devcoord = vertexpos * skewmatrix + translate;");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devcoord");

        // Setup interpolants for coverage.
        GrGLSLVarying arcCoord(useHWDerivatives ? kFloat2_GrSLType : kFloat4_GrSLType);
        varyings->addVarying("arccoord", &arcCoord);
        v->codeAppend("if (0 != is_linear_coverage) {");
                           // We are a non-corner piece: Set x=0 to indicate built-in coverage, and
                           // interpolate linear coverage across y.
        v->codeAppendf(    "%s.xy = float2(0, coverage * coverage_multiplier);",
                           arcCoord.vsOut());
        v->codeAppend("} else {");
                           // Find the normalized arc coordinates for our corner ellipse.
                           // (i.e., the coordinate system where x^2 + y^2 == 1).
        v->codeAppend(    "float2 arccoord = 1 - abs(radius_outset) + aa_outset/radii * corner;");
                           // We are a corner piece: Interpolate the arc coordinates for coverage.
                           // Emit x+1 to ensure no pixel in the arc has a x value of 0 (since x=0
                           // instructs the fragment shader to use linear coverage).
        v->codeAppendf(    "%s.xy = float2(arccoord.x+1, arccoord.y);", arcCoord.vsOut());
        if (!useHWDerivatives) {
            // The gradient is order-1: Interpolate it across arccoord.zw.
            v->codeAppendf("float2x2 derivatives = inverse(skewmatrix);");
            v->codeAppendf("%s.zw = derivatives * (arccoord/radii * 2);", arcCoord.vsOut());
        }
        v->codeAppend("}");

        // Emit the fragment shader.
        f->codeAppendf("float x_plus_1=%s.x, y=%s.y;", arcCoord.fsIn(), arcCoord.fsIn());
        f->codeAppendf("half coverage;");
        f->codeAppendf("if (0 == x_plus_1) {");
        f->codeAppendf(    "coverage = half(y);");  // We are a non-arc pixel (linear coverage).
        f->codeAppendf("} else {");
        f->codeAppendf(    "float fn = x_plus_1 * (x_plus_1 - 2);");  // fn = (x+1)*(x-1) = x^2-1
        f->codeAppendf(    "fn = fma(y,y, fn);");  // fn = x^2 + y^2 - 1
        if (useHWDerivatives) {
            f->codeAppendf("float fnwidth = fwidth(fn);");
        } else {
            // The gradient is interpolated across arccoord.zw.
            f->codeAppendf("float gx=%s.z, gy=%s.w;", arcCoord.fsIn(), arcCoord.fsIn());
            f->codeAppendf("float fnwidth = abs(gx) + abs(gy);");
        }
        f->codeAppendf(    "coverage = .5 - half(fn/fnwidth);");
        if (proc.fFlags & ProcessorFlags::kMSAAEnabled) {
            // MSAA uses ramps larger than 1px, so we need to clamp in both branches.
            f->codeAppendf("}");
        }
        f->codeAppendf("coverage = clamp(coverage, 0, 1);");
        if (!(proc.fFlags & ProcessorFlags::kMSAAEnabled)) {
            // When not using MSAA, we only need to clamp in the "arc" branch.
            f->codeAppendf("}");
        }
        if (proc.fFlags & ProcessorFlags::kFakeNonAA) {
            f->codeAppendf("coverage = (coverage >= .5) ? 1 : 0;");
        }
        f->codeAppendf("half4 %s = half4(coverage);", args.fOutputCoverage);
    }
};

std::unique_ptr<GrGeometryProcessor::ProgramImpl> FillRRectOpImpl::Processor::makeProgramImpl(
        const GrShaderCaps&) const {
    return std::make_unique<Impl>();
}

void FillRRectOpImpl::onCreateProgramInfo(const GrCaps* caps,
                                          SkArenaAlloc* arena,
                                          const GrSurfaceProxyView& writeView,
                                          bool usesMSAASurface,
                                          GrAppliedClip&& appliedClip,
                                          const GrDstProxyView& dstProxyView,
                                          GrXferBarrierFlags renderPassXferBarriers,
                                          GrLoadOp colorLoadOp) {
    if (usesMSAASurface) {
        fProcessorFlags |= ProcessorFlags::kMSAAEnabled;
    }
    GrGeometryProcessor* gp = Processor::Make(arena, fHelper.aaType(), fProcessorFlags);
    fProgramInfo = fHelper.createProgramInfo(caps, arena, writeView, usesMSAASurface,
                                             std::move(appliedClip), dstProxyView, gp,
                                             GrPrimitiveType::kTriangles, renderPassXferBarriers,
                                             colorLoadOp);
}

void FillRRectOpImpl::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fInstanceBuffer || !fIndexBuffer || !fVertexBuffer) {
        return;  // Setup failed.
    }

    flushState->bindPipelineAndScissorClip(*fProgramInfo, this->bounds());
    flushState->bindTextures(fProgramInfo->geomProc(), nullptr, fProgramInfo->pipeline());
    flushState->bindBuffers(std::move(fIndexBuffer), std::move(fInstanceBuffer),
                            std::move(fVertexBuffer));
    flushState->drawIndexedInstanced(SK_ARRAY_COUNT(kIndexData), 0, fInstanceCount, fBaseInstance,
                                     0);
}

// Will the given corner look good if we use HW derivatives?
bool can_use_hw_derivatives_with_coverage(const Sk2f& devScale, const Sk2f& cornerRadii) {
    Sk2f devRadii = devScale * cornerRadii;
    if (devRadii[1] < devRadii[0]) {
        devRadii = SkNx_shuffle<1,0>(devRadii);
    }
    float minDevRadius = std::max(devRadii[0], 1.f);  // Shader clamps radius at a minimum of 1.
    // Is the gradient smooth enough for this corner look ok if we use hardware derivatives?
    // This threshold was arrived at subjevtively on an NVIDIA chip.
    return minDevRadius * minDevRadius * 5 > devRadii[1];
}

bool can_use_hw_derivatives_with_coverage(const Sk2f& devScale, const SkVector& cornerRadii) {
    return can_use_hw_derivatives_with_coverage(devScale, Sk2f::Load(&cornerRadii));
}

// Will the given round rect look good if we use HW derivatives?
bool can_use_hw_derivatives_with_coverage(const GrShaderCaps& shaderCaps,
                                          const SkMatrix& viewMatrix,
                                          const SkRRect& rrect) {
    if (!shaderCaps.shaderDerivativeSupport()) {
        return false;
    }

    Sk2f x = Sk2f(viewMatrix.getScaleX(), viewMatrix.getSkewX());
    Sk2f y = Sk2f(viewMatrix.getSkewY(), viewMatrix.getScaleY());
    Sk2f devScale = (x*x + y*y).sqrt();
    switch (rrect.getType()) {
        case SkRRect::kEmpty_Type:
        case SkRRect::kRect_Type:
            return true;

        case SkRRect::kOval_Type:
        case SkRRect::kSimple_Type:
            return can_use_hw_derivatives_with_coverage(devScale, rrect.getSimpleRadii());

        case SkRRect::kNinePatch_Type: {
            Sk2f r0 = Sk2f::Load(SkRRectPriv::GetRadiiArray(rrect));
            Sk2f r1 = Sk2f::Load(SkRRectPriv::GetRadiiArray(rrect) + 2);
            Sk2f minRadii = Sk2f::Min(r0, r1);
            Sk2f maxRadii = Sk2f::Max(r0, r1);
            return can_use_hw_derivatives_with_coverage(devScale, Sk2f(minRadii[0], maxRadii[1])) &&
                   can_use_hw_derivatives_with_coverage(devScale, Sk2f(maxRadii[0], minRadii[1]));
        }

        case SkRRect::kComplex_Type: {
            for (int i = 0; i < 4; ++i) {
                auto corner = static_cast<SkRRect::Corner>(i);
                if (!can_use_hw_derivatives_with_coverage(devScale, rrect.radii(corner))) {
                    return false;
                }
            }
            return true;
        }
    }
    SK_ABORT("Invalid round rect type.");
}

} // anonymous namespace

GrOp::Owner Make(GrRecordingContext* ctx,
                 SkArenaAlloc* arena,
                 GrPaint&& paint,
                 const SkMatrix& viewMatrix,
                 const SkRRect& rrect,
                 const SkRect& localRect,
                 GrAA aa) {
    return FillRRectOpImpl::Make(ctx, arena, std::move(paint), viewMatrix, rrect, localRect, aa);
}

} // namespace skgpu::v1::FillRRectOp

#if GR_TEST_UTILS

#include "src/gpu/GrDrawOpTest.h"

GR_DRAW_OP_TEST_DEFINE(FillRRectOp) {
    SkArenaAlloc arena(64 * sizeof(float));
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrAA aa = GrAA(random->nextBool());

    SkRect rect = GrTest::TestRect(random);
    float w = rect.width();
    float h = rect.height();

    SkRRect rrect;
    // TODO: test out other rrect configurations
    rrect.setNinePatch(rect, w / 3.0f, h / 4.0f, w / 5.0f, h / 6.0);

    return skgpu::v1::FillRRectOp::Make(context,
                                        &arena,
                                        std::move(paint),
                                        viewMatrix,
                                        rrect,
                                        rrect.rect(),
                                        aa);
}

#endif
