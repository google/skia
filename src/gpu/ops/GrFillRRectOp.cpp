/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrFillRRectOp.h"

#include "include/private/GrRecordingContext.h"
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

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext*,
                                          GrPaint&&,
                                          const SkMatrix& viewMatrix,
                                          const SkRRect&,
                                          GrAAType);

    const char* name() const final { return "GrFillRRectOp"; }

    FixedFunctionFlags fixedFunctionFlags() const final { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) final;
    CombineResult onCombineIfPossible(GrOp*, GrRecordingContext::Arenas*, const GrCaps&) final;

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
    friend class ::GrOpMemoryPool;         // for access to ctor

    enum class ProcessorFlags {
        kNone             = 0,
        kUseHWDerivatives = 1 << 0,
        kHasPerspective   = 1 << 1,
        kHasLocalCoords   = 1 << 2,
        kWideColor        = 1 << 3
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(ProcessorFlags);

    class Processor;

    FillRRectOp(const Helper::MakeArgs&,
                const SkPMColor4f& paintColor,
                const SkMatrix& totalShapeMatrix,
                const SkRRect&,
                GrAAType,
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
                             const GrSurfaceProxyView* outputView,
                             GrAppliedClip&&,
                             const GrXferProcessor::DstProxyView&) final;

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
    int fIndexCount = 0;

    // If this op is prePrepared the created programInfo will be stored here for use in
    // onExecute. In the prePrepared case it will have been stored in the record-time arena.
    GrProgramInfo* fProgramInfo = nullptr;

    typedef GrMeshDrawOp INHERITED;
};

GR_MAKE_BITFIELD_CLASS_OPS(FillRRectOp::ProcessorFlags)

// Hardware derivatives are not always accurate enough for highly elliptical corners. This method
// checks to make sure the corners will still all look good if we use HW derivatives.
static bool can_use_hw_derivatives_with_coverage(const GrShaderCaps&,
                                                 const SkMatrix&,
                                                 const SkRRect&);

std::unique_ptr<GrDrawOp> FillRRectOp::Make(GrRecordingContext* ctx,
                                            GrPaint&& paint,
                                            const SkMatrix& viewMatrix,
                                            const SkRRect& rrect,
                                            GrAAType aaType) {
    using Helper = GrSimpleMeshDrawOpHelper;

    const GrCaps* caps = ctx->priv().caps();

    if (!caps->instanceAttribSupport()) {
        return nullptr;
    }

    ProcessorFlags flags = ProcessorFlags::kNone;
    if (GrAAType::kCoverage == aaType) {
        // TODO: Support perspective in a follow-on CL. This shouldn't be difficult, since we
        // already use HW derivatives. The only trick will be adjusting the AA outset to account for
        // perspective. (i.e., outset = 0.5 * z.)
        if (viewMatrix.hasPerspective()) {
            return nullptr;
        }
        if (can_use_hw_derivatives_with_coverage(*caps->shaderCaps(), viewMatrix, rrect)) {
            // HW derivatives (more specifically, fwidth()) are consistently faster on all platforms
            // in coverage mode. We use them as long as the approximation will be accurate enough.
            flags |= ProcessorFlags::kUseHWDerivatives;
        }
    } else {
        if (GrAAType::kMSAA == aaType) {
            if (!caps->sampleLocationsSupport() || !caps->shaderCaps()->sampleMaskSupport() ||
                caps->shaderCaps()->canOnlyUseSampleMaskWithStencil()) {
                return nullptr;
            }
        }
        if (viewMatrix.hasPerspective()) {
            // HW derivatives are consistently slower on all platforms in sample mask mode. We
            // therefore only use them when there is perspective, since then we can't interpolate
            // the symbolic screen-space gradient.
            flags |= ProcessorFlags::kUseHWDerivatives | ProcessorFlags::kHasPerspective;
        }
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
    if (!(flags & ProcessorFlags::kHasPerspective)) {
        // Since m is an affine matrix that maps the rect [-1, -1, +1, +1] into the shape's
        // device-space quad, it's quite simple to find the bounding rectangle:
        devBounds = SkRect::MakeXYWH(m.getTranslateX(), m.getTranslateY(), 0, 0);
        devBounds.outset(SkScalarAbs(m.getScaleX()) + SkScalarAbs(m.getSkewX()),
                         SkScalarAbs(m.getSkewY()) + SkScalarAbs(m.getScaleY()));
    } else {
        viewMatrix.mapRect(&devBounds, rrect.rect());
    }

    if (GrAAType::kMSAA == aaType && caps->preferTrianglesOverSampleMask()) {
        // We are on a platform that prefers fine triangles instead of using the sample mask. See if
        // the round rect is large enough that it will be faster for us to send it off to the
        // default path renderer instead. The 200x200 threshold was arrived at using the
        // "shapes_rrect" benchmark on an ARM Galaxy S9.
        if (devBounds.height() * devBounds.width() > 200 * 200) {
            return nullptr;
        }
    }

    return Helper::FactoryHelper<FillRRectOp>(ctx, std::move(paint), m, rrect, aaType,
                                              flags, devBounds);
}

FillRRectOp::FillRRectOp(const GrSimpleMeshDrawOpHelper::MakeArgs& helperArgs,
                         const SkPMColor4f& paintColor,
                         const SkMatrix& totalShapeMatrix,
                         const SkRRect& rrect,
                         GrAAType aaType,
                         ProcessorFlags processorFlags,
                         const SkRect& devBounds)
        : INHERITED(ClassID())
        , fHelper(helperArgs, aaType)
        , fColor(paintColor)
        , fLocalRect(rrect.rect())
        , fProcessorFlags(processorFlags & ~(ProcessorFlags::kHasLocalCoords |
                                             ProcessorFlags::kWideColor)) {
    SkASSERT((fProcessorFlags & ProcessorFlags::kHasPerspective) ==
                                                                totalShapeMatrix.hasPerspective());
    this->setBounds(devBounds, GrOp::HasAABloat::kYes, GrOp::IsHairline::kNo);

    // Write the matrix attribs.
    const SkMatrix& m = totalShapeMatrix;
    if (!(fProcessorFlags & ProcessorFlags::kHasPerspective)) {
        // Affine 2D transformation (float2x2 plus float2 translate).
        SkASSERT(!m.hasPerspective());
        this->writeInstanceData(m.getScaleX(), m.getSkewX(), m.getSkewY(), m.getScaleY());
        this->writeInstanceData(m.getTranslateX(), m.getTranslateY());
    } else {
        // Perspective float3x3 transformation matrix.
        SkASSERT(m.hasPerspective());
        m.get9(this->appendInstanceData<float>(9));
    }

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

GrDrawOp::CombineResult FillRRectOp::onCombineIfPossible(GrOp* op,
                                                         GrRecordingContext::Arenas*,
                                                         const GrCaps& caps) {
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
        return arena->make<Processor>(aaType, flags);
    }

    const char* name() const final { return "GrFillRRectOp::Processor"; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const final {
        b->add32(((uint32_t)fFlags << 16) | (uint32_t)fAAType);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    friend class ::SkArenaAlloc; // for access to ctor

    Processor(GrAAType aaType, ProcessorFlags flags)
            : INHERITED(kGrFillRRectOp_Processor_ClassID)
            , fAAType(aaType)
            , fFlags(flags) {
        int numVertexAttribs = (GrAAType::kCoverage == fAAType) ? 3 : 2;
        this->setVertexAttributes(kVertexAttribs, numVertexAttribs);

        if (!(fFlags & ProcessorFlags::kHasPerspective)) {
            // Affine 2D transformation (float2x2 plus float2 translate).
            fInstanceAttribs.emplace_back("skew", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
            fInstanceAttribs.emplace_back(
                    "translate", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
        } else {
            // Perspective float3x3 transformation matrix.
            fInstanceAttribs.emplace_back("persp_x", kFloat3_GrVertexAttribType, kFloat3_GrSLType);
            fInstanceAttribs.emplace_back("persp_y", kFloat3_GrVertexAttribType, kFloat3_GrSLType);
            fInstanceAttribs.emplace_back("persp_z", kFloat3_GrVertexAttribType, kFloat3_GrSLType);
        }
        fInstanceAttribs.emplace_back("radii_x", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fInstanceAttribs.emplace_back("radii_y", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        fColorAttrib = &fInstanceAttribs.push_back(
                MakeColorAttribute("color", (fFlags & ProcessorFlags::kWideColor)));
        if (fFlags & ProcessorFlags::kHasLocalCoords) {
            fInstanceAttribs.emplace_back(
                    "local_rect", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
        }
        this->setInstanceAttributes(fInstanceAttribs.begin(), fInstanceAttribs.count());

        if (GrAAType::kMSAA == fAAType) {
            this->setWillUseCustomFeature(CustomFeatures::kSampleLocations);
        }
    }

    static constexpr Attribute kVertexAttribs[] = {
            {"radii_selector", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"corner_and_radius_outsets", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            // Coverage only.
            {"aa_bloat_and_coverage", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};

    const GrAAType       fAAType;
    const ProcessorFlags fFlags;

    SkSTArray<6, Attribute> fInstanceAttribs;
    const Attribute* fColorAttrib;

    class CoverageImpl;
    class MSAAImpl;

    typedef GrGeometryProcessor INHERITED;
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

static constexpr CoverageVertex kCoverageVertexData[] = {
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

GR_DECLARE_STATIC_UNIQUE_KEY(gCoverageVertexBufferKey);

static constexpr uint16_t kCoverageIndexData[] = {
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

GR_DECLARE_STATIC_UNIQUE_KEY(gCoverageIndexBufferKey);


// Our MSAA geometry consists of an inset octagon with full sample mask coverage, circumscribed
// by a larger octagon that modifies the sample mask for the arc at each corresponding corner.
struct MSAAVertex {
    std::array<float, 4> fRadiiSelector;
    std::array<float, 2> fCorner;
    std::array<float, 2> fRadiusOutset;
};

static constexpr MSAAVertex kMSAAVertexData[] = {
        // Left edge. (Negative radii selector indicates this is not an arc section.)
        {{{0,0,0,-1}},  {{-1,+1}},  {{0,-1}}},
        {{{-1,0,0,0}},  {{-1,-1}},  {{0,+1}}},

        // Top edge.
        {{{-1,0,0,0}},  {{-1,-1}},  {{+1,0}}},
        {{{0,-1,0,0}},  {{+1,-1}},  {{-1,0}}},

        // Right edge.
        {{{0,-1,0,0}},  {{+1,-1}},  {{0,+1}}},
        {{{0,0,-1,0}},  {{+1,+1}},  {{0,-1}}},

        // Bottom edge.
        {{{0,0,-1,0}},  {{+1,+1}},  {{-1,0}}},
        {{{0,0,0,-1}},  {{-1,+1}},  {{+1,0}}},

        // Top-left corner.
        {{{1,0,0,0}},  {{-1,-1}},  {{0,+1}}},
        {{{1,0,0,0}},  {{-1,-1}},  {{0,+kOctoOffset}}},
        {{{1,0,0,0}},  {{-1,-1}},  {{+1,0}}},
        {{{1,0,0,0}},  {{-1,-1}},  {{+kOctoOffset,0}}},

        // Top-right corner.
        {{{0,1,0,0}},  {{+1,-1}},  {{-1,0}}},
        {{{0,1,0,0}},  {{+1,-1}},  {{-kOctoOffset,0}}},
        {{{0,1,0,0}},  {{+1,-1}},  {{0,+1}}},
        {{{0,1,0,0}},  {{+1,-1}},  {{0,+kOctoOffset}}},

        // Bottom-right corner.
        {{{0,0,1,0}},  {{+1,+1}},  {{0,-1}}},
        {{{0,0,1,0}},  {{+1,+1}},  {{0,-kOctoOffset}}},
        {{{0,0,1,0}},  {{+1,+1}},  {{-1,0}}},
        {{{0,0,1,0}},  {{+1,+1}},  {{-kOctoOffset,0}}},

        // Bottom-left corner.
        {{{0,0,0,1}},  {{-1,+1}},  {{+1,0}}},
        {{{0,0,0,1}},  {{-1,+1}},  {{+kOctoOffset,0}}},
        {{{0,0,0,1}},  {{-1,+1}},  {{0,-1}}},
        {{{0,0,0,1}},  {{-1,+1}},  {{0,-kOctoOffset}}}};

GR_DECLARE_STATIC_UNIQUE_KEY(gMSAAVertexBufferKey);

static constexpr uint16_t kMSAAIndexData[] = {
        // Inset octagon. (Full sample mask.)
        0, 1, 2,
        0, 2, 3,
        0, 3, 6,
        3, 4, 5,
        3, 5, 6,
        6, 7, 0,

        // Top-left arc. (Sample mask is set to the arc.)
         8,  9, 10,
         9, 11, 10,

        // Top-right arc.
        12, 13, 14,
        13, 15, 14,

        // Bottom-right arc.
        16, 17, 18,
        17, 19, 18,

        // Bottom-left arc.
        20, 21, 22,
        21, 23, 22};

GR_DECLARE_STATIC_UNIQUE_KEY(gMSAAIndexBufferKey);

void FillRRectOp::onPrepareDraws(Target* target) {
    if (void* instanceData = target->makeVertexSpace(fInstanceStride, fInstanceCount,
                                                     &fInstanceBuffer, &fBaseInstance)) {
        SkASSERT(fInstanceStride * fInstanceCount == fInstanceData.count());
        memcpy(instanceData, fInstanceData.begin(), fInstanceData.count());
    }

    if (GrAAType::kCoverage == fHelper.aaType()) {
        GR_DEFINE_STATIC_UNIQUE_KEY(gCoverageIndexBufferKey);

        fIndexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kIndex, sizeof(kCoverageIndexData), kCoverageIndexData,
                gCoverageIndexBufferKey);

        GR_DEFINE_STATIC_UNIQUE_KEY(gCoverageVertexBufferKey);

        fVertexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex, sizeof(kCoverageVertexData), kCoverageVertexData,
                gCoverageVertexBufferKey);

        fIndexCount = SK_ARRAY_COUNT(kCoverageIndexData);
    } else {
        GR_DEFINE_STATIC_UNIQUE_KEY(gMSAAIndexBufferKey);

        fIndexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kIndex, sizeof(kMSAAIndexData), kMSAAIndexData,
                gMSAAIndexBufferKey);

        GR_DEFINE_STATIC_UNIQUE_KEY(gMSAAVertexBufferKey);

        fVertexBuffer = target->resourceProvider()->findOrMakeStaticBuffer(
                GrGpuBufferType::kVertex, sizeof(kMSAAVertexData), kMSAAVertexData,
                gMSAAVertexBufferKey);

        fIndexCount = SK_ARRAY_COUNT(kMSAAIndexData);
    }
}

class FillRRectOp::Processor::CoverageImpl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& proc = args.fGP.cast<Processor>();
        bool useHWDerivatives = (proc.fFlags & ProcessorFlags::kUseHWDerivatives);

        SkASSERT(proc.vertexStride() == sizeof(CoverageVertex));

        GrGLSLVaryingHandler* varyings = args.fVaryingHandler;
        varyings->emitAttributes(proc);
        varyings->addPassThroughAttribute(*proc.fColorAttrib, args.fOutputColor,
                                          GrGLSLVaryingHandler::Interpolation::kCanBeFlat);

        // Emit the vertex shader.
        GrGLSLVertexBuilder* v = args.fVertBuilder;

        // Unpack vertex attribs.
        v->codeAppend("float2 corner = corner_and_radius_outsets.xy;");
        v->codeAppend("float2 radius_outset = corner_and_radius_outsets.zw;");
        v->codeAppend("float2 aa_bloat_direction = aa_bloat_and_coverage.xy;");
        v->codeAppend("float coverage = aa_bloat_and_coverage.z;");
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

        v->codeAppend("if (any(greaterThan(aa_bloatradius, float2(1)))) {");
                          // The rrect is more narrow than an AA coverage ramp. We can't draw as-is
                          // or else opposite AA borders will overlap. Instead, fudge the size up to
                          // the width of a coverage ramp, and then reduce total coverage to make
                          // the rect appear more thin.
        v->codeAppend(    "corner = max(abs(corner), aa_bloatradius) * sign(corner);");
        v->codeAppend(    "coverage /= max(aa_bloatradius.x, 1) * max(aa_bloatradius.y, 1);");
                          // Set radii to zero to ensure we take the "linear coverage" codepath.
                          // (The "coverage" variable only has effect in the linear codepath.)
        v->codeAppend(    "radii = float2(0);");
        v->codeAppend("}");

        v->codeAppend("if (any(lessThan(radii, aa_bloatradius * 1.25))) {");
                          // The radii are very small. Demote this arc to a sharp 90 degree corner.
        v->codeAppend(    "radii = aa_bloatradius;");
                          // Snap octagon vertices to the corner of the bounding box.
        v->codeAppend(    "radius_outset = floor(abs(radius_outset)) * radius_outset;");
        v->codeAppend(    "is_linear_coverage = 1;");
        v->codeAppend("} else {");
                          // Don't let radii get smaller than a pixel.
        v->codeAppend(    "radii = clamp(radii, pixellength, 2 - pixellength);");
        v->codeAppend(    "neighbor_radii = clamp(neighbor_radii, pixellength, 2 - pixellength);");
                          // Don't let neighboring radii get closer together than 1/16 pixel.
        v->codeAppend(    "float2 spacing = 2 - radii - neighbor_radii;");
        v->codeAppend(    "float2 extra_pad = max(pixellength * .0625 - spacing, float2(0));");
        v->codeAppend(    "radii -= extra_pad * .5;");
        v->codeAppend("}");

        // Find our vertex position, adjusted for radii and bloated for AA. Our rect is drawn in
        // normalized [-1,-1,+1,+1] space.
        v->codeAppend("float2 aa_outset = aa_bloat_direction.xy * aa_bloatradius;");
        v->codeAppend("float2 vertexpos = corner + radius_outset * radii + aa_outset;");

        // Emit transforms.
        GrShaderVar localCoord("", kFloat2_GrSLType);
        if (proc.fFlags & ProcessorFlags::kHasLocalCoords) {
            v->codeAppend("float2 localcoord = (local_rect.xy * (1 - vertexpos) + "
                                               "local_rect.zw * (1 + vertexpos)) * .5;");
            localCoord.set(kFloat2_GrSLType, "localcoord");
        }
        this->emitTransforms(v, varyings, args.fUniformHandler, localCoord,
                             args.fFPCoordTransformHandler);

        // Transform to device space.
        SkASSERT(!(proc.fFlags & ProcessorFlags::kHasPerspective));
        v->codeAppend("float2x2 skewmatrix = float2x2(skew.xy, skew.zw);");
        v->codeAppend("float2 devcoord = vertexpos * skewmatrix + translate;");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devcoord");

        // Setup interpolants for coverage.
        GrGLSLVarying arcCoord(useHWDerivatives ? kFloat2_GrSLType : kFloat4_GrSLType);
        varyings->addVarying("arccoord", &arcCoord);
        v->codeAppend("if (0 != is_linear_coverage) {");
                           // We are a non-corner piece: Set x=0 to indicate built-in coverage, and
                           // interpolate linear coverage across y.
        v->codeAppendf(    "%s.xy = float2(0, coverage);", arcCoord.vsOut());
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
        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

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
        f->codeAppendf(    "half d = half(fn/fnwidth);");
        f->codeAppendf(    "coverage = clamp(.5 - d, 0, 1);");
        f->codeAppendf("}");
        f->codeAppendf("%s = half4(coverage);", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 const CoordTransformRange& transformRange) override {
        this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
    }
};


class FillRRectOp::Processor::MSAAImpl : public GrGLSLGeometryProcessor {
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& proc = args.fGP.cast<Processor>();
        bool useHWDerivatives = (proc.fFlags & ProcessorFlags::kUseHWDerivatives);
        bool hasPerspective = (proc.fFlags & ProcessorFlags::kHasPerspective);
        bool hasLocalCoords = (proc.fFlags & ProcessorFlags::kHasLocalCoords);
        SkASSERT(useHWDerivatives == hasPerspective);

        SkASSERT(proc.vertexStride() == sizeof(MSAAVertex));

        // Emit the vertex shader.
        GrGLSLVertexBuilder* v = args.fVertBuilder;

        GrGLSLVaryingHandler* varyings = args.fVaryingHandler;
        varyings->emitAttributes(proc);
        varyings->addPassThroughAttribute(*proc.fColorAttrib, args.fOutputColor,
                                          GrGLSLVaryingHandler::Interpolation::kCanBeFlat);

        // Unpack vertex attribs.
        v->codeAppendf("float2 corner = corner_and_radius_outsets.xy;");
        v->codeAppendf("float2 radius_outset = corner_and_radius_outsets.zw;");

        // Identify our radii.
        v->codeAppend("float2 radii;");
        v->codeAppend("radii.x = dot(radii_selector, radii_x);");
        v->codeAppend("radii.y = dot(radii_selector, radii_y);");
        v->codeAppendf("bool is_arc_section = (radii.x > 0);");
        v->codeAppendf("radii = abs(radii);");

        // Find our vertex position, adjusted for radii. Our rect is drawn in normalized
        // [-1,-1,+1,+1] space.
        v->codeAppend("float2 vertexpos = corner + radius_outset * radii;");

        // Emit transforms.
        GrShaderVar localCoord("", kFloat2_GrSLType);
        if (hasLocalCoords) {
            v->codeAppend("float2 localcoord = (local_rect.xy * (1 - vertexpos) + "
                                               "local_rect.zw * (1 + vertexpos)) * .5;");
            localCoord.set(kFloat2_GrSLType, "localcoord");
        }
        this->emitTransforms(v, varyings, args.fUniformHandler, localCoord,
                             args.fFPCoordTransformHandler);

        // Transform to device space.
        if (!hasPerspective) {
            v->codeAppend("float2x2 skewmatrix = float2x2(skew.xy, skew.zw);");
            v->codeAppend("float2 devcoord = vertexpos * skewmatrix + translate;");
            gpArgs->fPositionVar.set(kFloat2_GrSLType, "devcoord");
        } else {
            v->codeAppend("float3x3 persp_matrix = float3x3(persp_x, persp_y, persp_z);");
            v->codeAppend("float3 devcoord = float3(vertexpos, 1) * persp_matrix;");
            gpArgs->fPositionVar.set(kFloat3_GrSLType, "devcoord");
        }

        // Determine normalized arc coordinates for the implicit function.
        GrGLSLVarying arcCoord((useHWDerivatives) ? kFloat2_GrSLType : kFloat4_GrSLType);
        varyings->addVarying("arccoord", &arcCoord);
        v->codeAppendf("if (is_arc_section) {");
        v->codeAppendf(    "%s.xy = 1 - abs(radius_outset);", arcCoord.vsOut());
        if (!useHWDerivatives) {
            // The gradient is order-1: Interpolate it across arccoord.zw.
            // This doesn't work with perspective.
            SkASSERT(!hasPerspective);
            v->codeAppendf("float2x2 derivatives = inverse(skewmatrix);");
            v->codeAppendf("%s.zw = derivatives * (%s.xy/radii * corner * 2);",
                           arcCoord.vsOut(), arcCoord.vsOut());
        }
        v->codeAppendf("} else {");
        if (useHWDerivatives) {
            v->codeAppendf("%s = float2(0);", arcCoord.vsOut());
        } else {
            v->codeAppendf("%s = float4(0);", arcCoord.vsOut());
        }
        v->codeAppendf("}");

        // Emit the fragment shader.
        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

        f->codeAppendf("%s = half4(1);", args.fOutputCoverage);

        // If x,y == 0, then we are drawing a triangle that does not track an arc.
        f->codeAppendf("if (float2(0) != %s.xy) {", arcCoord.fsIn());
        f->codeAppendf(    "float fn = dot(%s.xy, %s.xy) - 1;", arcCoord.fsIn(), arcCoord.fsIn());
        if (GrAAType::kMSAA == proc.fAAType) {
            using ScopeFlags = GrGLSLFPFragmentBuilder::ScopeFlags;
            if (!useHWDerivatives) {
                f->codeAppendf("float2 grad = %s.zw;", arcCoord.fsIn());
                f->applyFnToMultisampleMask("fn", "grad", ScopeFlags::kInsidePerPrimitiveBranch);
            } else {
                f->applyFnToMultisampleMask("fn", nullptr, ScopeFlags::kInsidePerPrimitiveBranch);
            }
        } else {
            f->codeAppendf("if (fn > 0) {");
            f->codeAppendf(    "%s = half4(0);", args.fOutputCoverage);
            f->codeAppendf("}");
        }
        f->codeAppendf("}");
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 const CoordTransformRange& transformRange) override {
        this->setTransformDataHelper(SkMatrix::I(), pdman, transformRange);
    }
};

GrGLSLPrimitiveProcessor* FillRRectOp::Processor::createGLSLInstance(
        const GrShaderCaps&) const {
    if (GrAAType::kCoverage != fAAType) {
        return new MSAAImpl();
    }
    return new CoverageImpl();
}

void FillRRectOp::onCreateProgramInfo(const GrCaps* caps,
                                      SkArenaAlloc* arena,
                                      const GrSurfaceProxyView* outputView,
                                      GrAppliedClip&& appliedClip,
                                      const GrXferProcessor::DstProxyView& dstProxyView) {
    GrGeometryProcessor* gp = Processor::Make(arena, fHelper.aaType(), fProcessorFlags);
    SkASSERT(gp->instanceStride() == (size_t)fInstanceStride);

    fProgramInfo = fHelper.createProgramInfo(caps, arena, outputView, std::move(appliedClip),
                                             dstProxyView, gp, GrPrimitiveType::kTriangles);
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
    flushState->bindBuffers(fIndexBuffer.get(), fInstanceBuffer.get(), fVertexBuffer.get());
    flushState->drawIndexedInstanced(fIndexCount, 0, fInstanceCount, fBaseInstance, 0);
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


std::unique_ptr<GrDrawOp> GrFillRRectOp::Make(GrRecordingContext* ctx,
                                              GrPaint&& paint,
                                              const SkMatrix& viewMatrix,
                                              const SkRRect& rrect,
                                              GrAAType aaType) {
    return FillRRectOp::Make(ctx, std::move(paint), viewMatrix, rrect, aaType);
}

#if GR_TEST_UTILS

#include "src/gpu/GrDrawOpTest.h"

GR_DRAW_OP_TEST_DEFINE(FillRRectOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrAAType aaType = GrAAType::kNone;
    if (random->nextBool()) {
        aaType = (numSamples > 1) ? GrAAType::kMSAA : GrAAType::kCoverage;
    }

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
                               aaType);
}

#endif
