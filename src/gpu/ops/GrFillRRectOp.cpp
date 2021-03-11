/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrFillRRectOp.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

namespace {

class FillRRectOp : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(GrRecordingContext*,
                            GrPaint&&,
                            const SkMatrix& viewMatrix,
                            const SkRRect&,
                            GrAA);

    const char* name() const final { return "GrFillRRectOp"; }

    FixedFunctionFlags fixedFunctionFlags() const final { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) final;
    CombineResult onCombineIfPossible(GrOp*, SkArenaAlloc*, const GrCaps&) final;

    void visitProxies(const VisitProxyFunc& fn) const override {
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(fn);
        } else {
            fHelper.visitProxies(fn);
        }
    }

    void onPrepareDraws(Target*) final;

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) final;

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

    FillRRectOp(GrProcessorSet*,
                const SkPMColor4f& paintColor,
                const SkMatrix& totalShapeMatrix,
                const SkRRect&,
                ProcessorFlags,
                const SkRect& devBounds);

    // These methods are used to append data of various POD types to our internal array of instance
    // data. The actual layout of the instance buffer can vary from Op to Op.
    template <typename T> inline T* appendInstanceData(int count) {
        static_assert(std::is_pod<T>::value, "");
        static_assert(4 == alignof(T), "");
        return reinterpret_cast<T*>(fInstanceData.push_back_n(sizeof(T) * count));
    }

    template <typename T, typename... Args>
    inline void writeInstanceData(const T& val, const Args&... remainder) {
        memcpy(this->appendInstanceData<T>(1), &val, sizeof(T));
        this->writeInstanceData(remainder...);
    }

    void writeInstanceData() {}  // Halt condition.

    GrProgramInfo* programInfo() final { return fProgramInfo; }

    // Create a GrProgramInfo object in the provided arena
    void onCreateProgramInfo(const GrCaps*,
                             SkArenaAlloc*,
                             const GrSurfaceProxyView& writeView,
                             GrAppliedClip&&,
                             const GrXferProcessor::DstProxyView&,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) final;

    Helper         fHelper;
    SkPMColor4f    fColor;
    const SkRect   fLocalRect;
    ProcessorFlags fProcessorFlags;

    SkSTArray<sizeof(float) * 16 * 4, char, /*MEM_MOVE=*/ true> fInstanceData;
    int fInstanceCount = 1;
    int fInstanceStride = 0;

    sk_sp<const GrBuffer> fInstanceBuffer;
    sk_sp<const GrBuffer> fVertexBuffer;
    sk_sp<const GrBuffer> fIndexBuffer;
    int fBaseInstance = 0;

    // If this op is prePrepared the created programInfo will be stored here for use in
    // onExecute. In the prePrepared case it will have been stored in the record-time arena.
    GrProgramInfo* fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

GR_MAKE_BITFIELD_CLASS_OPS(FillRRectOp::ProcessorFlags)

// Hardware derivatives are not always accurate enough for highly elliptical corners. This method
// checks to make sure the corners will still all look good if we use HW derivatives.
static bool can_use_hw_derivatives_with_coverage(const GrShaderCaps&,
                                                 const SkMatrix&,
                                                 const SkRRect&);

GrOp::Owner FillRRectOp::Make(GrRecordingContext* ctx,
                              GrPaint&& paint,
                              const SkMatrix& viewMatrix,
                              const SkRRect& rrect,
                              GrAA aa) {
    using Helper = GrSimpleMeshDrawOpHelper;

    const GrCaps* caps = ctx->priv().caps();

    if (!caps->drawInstancedSupport()) {
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

    // Produce a matrix that draws the round rect from normalized [-1, -1, +1, +1] space.
    float l = rrect.rect().left(), r = rrect.rect().right(),
          t = rrect.rect().top(), b = rrect.rect().bottom();
    SkMatrix m;
    // Unmap the normalized rect [-1, -1, +1, +1] back to [l, t, r, b].
    m.setScaleTranslate((r - l)/2, (b - t)/2, (l + r)/2, (t + b)/2);
    // Map to device space.
    m.postConcat(viewMatrix);

    SkRect devBounds;
    // Since m is an affine matrix that maps the rect [-1, -1, +1, +1] into the shape's
    // device-space quad, it's quite simple to find the bounding rectangle:
    devBounds = SkRect::MakeXYWH(m.getTranslateX(), m.getTranslateY(), 0, 0);
    devBounds.outset(SkScalarAbs(m.getScaleX()) + SkScalarAbs(m.getSkewX()),
                     SkScalarAbs(m.getSkewY()) + SkScalarAbs(m.getScaleY()));

    return Helper::FactoryHelper<FillRRectOp>(ctx, std::move(paint), m, rrect, flags, devBounds);
}

FillRRectOp::FillRRectOp(GrProcessorSet* processorSet,
                         const SkPMColor4f& paintColor,
                         const SkMatrix& totalShapeMatrix,
                         const SkRRect& rrect,
                         ProcessorFlags processorFlags,
                         const SkRect& devBounds)
        : INHERITED(ClassID())
        , fHelper(processorSet,
                  (processorFlags & ProcessorFlags::kFakeNonAA)
                          ? GrAAType::kNone
                          : GrAAType::kCoverage)  // Use analytic AA even if the RT is MSAA.
        , fColor(paintColor)
        , fLocalRect(rrect.rect())
        , fProcessorFlags(processorFlags & ~(ProcessorFlags::kHasLocalCoords |
                                             ProcessorFlags::kWideColor |
                                             ProcessorFlags::kMSAAEnabled)) {
    // FillRRectOp::Make fails if there is perspective.
    SkASSERT(!totalShapeMatrix.hasPerspective());
    this->setBounds(devBounds, GrOp::HasAABloat(!(processorFlags & ProcessorFlags::kFakeNonAA)),
                    GrOp::IsHairline::kNo);

    // Write the matrix attribs.
    const SkMatrix& m = totalShapeMatrix;
    // Affine 2D transformation (float2x2 plus float2 translate).
    SkASSERT(!m.hasPerspective());
    this->writeInstanceData(m.getScaleX(), m.getSkewX(), m.getSkewY(), m.getScaleY());
    this->writeInstanceData(m.getTranslateX(), m.getTranslateY());

    // Convert the radii to [-1, -1, +1, +1] space and write their attribs.
    Sk4f radiiX, radiiY;
    Sk4f::Load2(SkRRectPriv::GetRadiiArray(rrect), &radiiX, &radiiY);
    (radiiX * (2/rrect.width())).store(this->appendInstanceData<float>(4));
    (radiiY * (2/rrect.height())).store(this->appendInstanceData<float>(4));

    // We will write the color and local rect attribs during finalize().
}

GrProcessorSet::Analysis FillRRectOp::finalize(
        const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
        GrClampType clampType) {
    SkASSERT(1 == fInstanceCount);

    bool isWideColor;
    auto analysis = fHelper.finalizeProcessors(caps, clip, hasMixedSampledCoverage, clampType,
                                               GrProcessorAnalysisCoverage::kSingleChannel,
                                               &fColor, &isWideColor);

    // Finish writing the instance attribs.
    if (isWideColor) {
        fProcessorFlags |= ProcessorFlags::kWideColor;
        this->writeInstanceData(fColor);
    } else {
        this->writeInstanceData(fColor.toBytes_RGBA());
    }

    if (analysis.usesLocalCoords()) {
        fProcessorFlags |= ProcessorFlags::kHasLocalCoords;
        this->writeInstanceData(fLocalRect);
    }
    fInstanceStride = fInstanceData.count();

    return analysis;
}

GrOp::CombineResult FillRRectOp::onCombineIfPossible(GrOp* op, SkArenaAlloc*, const GrCaps& caps) {
    const auto& that = *op->cast<FillRRectOp>();
    if (!fHelper.isCompatible(that.fHelper, caps, this->bounds(), that.bounds())) {
        return CombineResult::kCannotCombine;
    }

    if (fProcessorFlags != that.fProcessorFlags ||
        fInstanceData.count() > std::numeric_limits<int>::max() - that.fInstanceData.count()) {
        return CombineResult::kCannotCombine;
    }

    fInstanceData.push_back_n(that.fInstanceData.count(), that.fInstanceData.begin());
    fInstanceCount += that.fInstanceCount;
    SkASSERT(fInstanceStride == that.fInstanceStride);
    return CombineResult::kMerged;
}

class FillRRectOp::Processor : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Make(SkArenaAlloc* arena, GrAAType aaType, ProcessorFlags flags) {
        return arena->make([&](void* ptr) {
            return new (ptr) Processor(aaType, flags);
        });
    }

    const char* name() const final { return "GrFillRRectOp::Processor"; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const final {
        b->addBits(kNumProcessorFlags, (uint32_t)fFlags,  "flags");
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    Processor(GrAAType aaType, ProcessorFlags flags)
            : INHERITED(kGrFillRRectOp_Processor_ClassID)
            , fFlags(flags) {
        this->setVertexAttributes(kVertexAttribs, SK_ARRAY_COUNT(kVertexAttribs));

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
        this->setInstanceAttributes(fInstanceAttribs.begin(), fInstanceAttribs.count());
    }

    static constexpr Attribute kVertexAttribs[] = {
            {"radii_selector", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"corner_and_radius_outsets", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            // Coverage only.
            {"aa_bloat_and_coverage", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};

    const ProcessorFlags fFlags;

    SkSTArray<6, Attribute> fInstanceAttribs;
    const Attribute* fColorAttrib;

    class Impl;

    using INHERITED = GrGeometryProcessor;
};

constexpr GrPrimitiveProcessor::Attribute FillRRectOp::Processor::kVertexAttribs[];

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

GR_DECLARE_STATIC_UNIQUE_KEY(gVertexBufferKey);

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

GR_DECLARE_STATIC_UNIQUE_KEY(gIndexBufferKey);

void FillRRectOp::onPrepareDraws(Target* target) {
    // We request no multisample, but some platforms don't support disabling it on MSAA targets.
    if (target->rtProxy()->numSamples() > 1 && !target->caps().multisampleDisableSupport()) {
        fProcessorFlags |= ProcessorFlags::kMSAAEnabled;
    }

    if (void* instanceData = target->makeVertexSpace(fInstanceStride, fInstanceCount,
                                                     &fInstanceBuffer, &fBaseInstance)) {
        SkASSERT(fInstanceStride * fInstanceCount == fInstanceData.count());
        memcpy(instanceData, fInstanceData.begin(), fInstanceData.count());
    }

    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);

    fIndexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(GrGpuBufferType::kIndex,
                                                                      sizeof(kIndexData),
                                                                      kIndexData, gIndexBufferKey);

    GR_DEFINE_STATIC_UNIQUE_KEY(gVertexBufferKey);

    fVertexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(GrGpuBufferType::kVertex,
                                                                      sizeof(kVertexData),
                                                                      kVertexData,
                                                                      gVertexBufferKey);
}

class FillRRectOp::Processor::Impl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        GrGLSLVertexBuilder* v = args.fVertBuilder;
        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

        const auto& proc = args.fGP.cast<Processor>();
        bool useHWDerivatives = (proc.fFlags & ProcessorFlags::kUseHWDerivatives);

        SkASSERT(proc.vertexStride() == sizeof(CoverageVertex));

        GrGLSLVaryingHandler* varyings = args.fVaryingHandler;
        varyings->emitAttributes(proc);
        f->codeAppendf("half4 %s;", args.fOutputColor);
        varyings->addPassThroughAttribute(*proc.fColorAttrib, args.fOutputColor,
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

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&) override {}
};


GrGLSLPrimitiveProcessor* FillRRectOp::Processor::createGLSLInstance(
        const GrShaderCaps&) const {
    return new Impl();
}

void FillRRectOp::onCreateProgramInfo(const GrCaps* caps,
                                      SkArenaAlloc* arena,
                                      const GrSurfaceProxyView& writeView,
                                      GrAppliedClip&& appliedClip,
                                      const GrXferProcessor::DstProxyView& dstProxyView,
                                      GrXferBarrierFlags renderPassXferBarriers,
                                      GrLoadOp colorLoadOp) {
    GrGeometryProcessor* gp = Processor::Make(arena, fHelper.aaType(), fProcessorFlags);
    SkASSERT(gp->instanceStride() == (size_t)fInstanceStride);

    fProgramInfo = fHelper.createProgramInfo(caps, arena, writeView, std::move(appliedClip),
                                             dstProxyView, gp, GrPrimitiveType::kTriangles,
                                             renderPassXferBarriers, colorLoadOp);
}

void FillRRectOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fInstanceBuffer || !fIndexBuffer || !fVertexBuffer) {
        return;  // Setup failed.
    }

    if (!fProgramInfo) {
        this->createProgramInfo(flushState);
    }

    flushState->bindPipelineAndScissorClip(*fProgramInfo, this->bounds());
    flushState->bindTextures(fProgramInfo->primProc(), nullptr, fProgramInfo->pipeline());
    flushState->bindBuffers(std::move(fIndexBuffer), std::move(fInstanceBuffer),
                            std::move(fVertexBuffer));
    flushState->drawIndexedInstanced(SK_ARRAY_COUNT(kIndexData), 0, fInstanceCount, fBaseInstance,
                                     0);
}

// Will the given corner look good if we use HW derivatives?
static bool can_use_hw_derivatives_with_coverage(const Sk2f& devScale, const Sk2f& cornerRadii) {
    Sk2f devRadii = devScale * cornerRadii;
    if (devRadii[1] < devRadii[0]) {
        devRadii = SkNx_shuffle<1,0>(devRadii);
    }
    float minDevRadius = std::max(devRadii[0], 1.f);  // Shader clamps radius at a minimum of 1.
    // Is the gradient smooth enough for this corner look ok if we use hardware derivatives?
    // This threshold was arrived at subjevtively on an NVIDIA chip.
    return minDevRadius * minDevRadius * 5 > devRadii[1];
}

static bool can_use_hw_derivatives_with_coverage(
        const Sk2f& devScale, const SkVector& cornerRadii) {
    return can_use_hw_derivatives_with_coverage(devScale, Sk2f::Load(&cornerRadii));
}

// Will the given round rect look good if we use HW derivatives?
static bool can_use_hw_derivatives_with_coverage(
        const GrShaderCaps& shaderCaps, const SkMatrix& viewMatrix, const SkRRect& rrect) {
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


GrOp::Owner GrFillRRectOp::Make(GrRecordingContext* ctx,
                                GrPaint&& paint,
                                const SkMatrix& viewMatrix,
                                const SkRRect& rrect,
                                GrAA aa) {
    return FillRRectOp::Make(ctx, std::move(paint), viewMatrix, rrect, aa);
}

#if GR_TEST_UTILS

#include "src/gpu/GrDrawOpTest.h"

GR_DRAW_OP_TEST_DEFINE(FillRRectOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrAA aa = GrAA(random->nextBool());

    SkRect rect = GrTest::TestRect(random);
    float w = rect.width();
    float h = rect.height();

    SkRRect rrect;
    // TODO: test out other rrect configurations
    rrect.setNinePatch(rect, w / 3.0f, h / 4.0f, w / 5.0f, h / 6.0);

    return GrFillRRectOp::Make(context,
                               std::move(paint),
                               viewMatrix,
                               rrect,
                               aa);
}

#endif
