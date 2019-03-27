/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrFillRRectOp.h"

#include "GrCaps.h"
#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "SkRRectPriv.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

// Hardware derivatives are not always accurate enough for highly elliptical corners. This method
// checks to make sure the corners will still all look good if we use HW derivatives.
static bool can_use_hw_derivatives(const GrShaderCaps&, const SkMatrix&, const SkRRect&);

std::unique_ptr<GrFillRRectOp> GrFillRRectOp::Make(
        GrRecordingContext* ctx, const SkMatrix& viewMatrix, const SkRRect& rrect,
        const GrCaps& caps, GrPaint&& paint) {
    if (!caps.instanceAttribSupport()) {
        return nullptr;
    }

    // TODO: Support perspective in a follow-on CL. This shouldn't be difficult, since we already
    // use HW derivatives. The only trick will be adjusting the AA outset to account for
    // perspective.  (i.e., outset = 0.5 * z.)
    if (viewMatrix.hasPerspective()) {
        return nullptr;
    }

    GrOpMemoryPool* pool = ctx->priv().opMemoryPool();
    return pool->allocate<GrFillRRectOp>(*caps.shaderCaps(), viewMatrix, rrect, std::move(paint));
}

GrFillRRectOp::GrFillRRectOp(const GrShaderCaps& shaderCaps, const SkMatrix& viewMatrix,
                             const SkRRect& rrect, GrPaint&& paint)
        : GrDrawOp(ClassID())
        , fOriginalColor(paint.getColor4f())
        , fLocalRect(rrect.rect())
        , fProcessors(std::move(paint)) {
    if (can_use_hw_derivatives(shaderCaps, viewMatrix, rrect)) {
        fFlags |= Flags::kUseHWDerivatives;
    }

    // Produce a matrix that draws the round rect from normalized [-1, -1, +1, +1] space.
    float l = rrect.rect().left(), r = rrect.rect().right(),
          t = rrect.rect().top(), b = rrect.rect().bottom();
    SkMatrix m;
    // Unmap the normalized rect [-1, -1, +1, +1] back to [l, t, r, b].
    m.setScaleTranslate((r - l)/2, (b - t)/2, (l + r)/2, (t + b)/2);
    // Map to device space.
    m.postConcat(viewMatrix);

    // Since m is an affine matrix that maps the rect [-1, -1, +1, +1] into the shape's
    // device-space quad, it's quite simple to find the bounding rectangle:
    SkASSERT(!m.hasPerspective());
    SkRect bounds = SkRect::MakeXYWH(m.getTranslateX(), m.getTranslateY(), 0, 0);
    bounds.outset(SkScalarAbs(m.getScaleX()) + SkScalarAbs(m.getSkewX()),
                  SkScalarAbs(m.getSkewY()) + SkScalarAbs(m.getScaleY()));
    this->setBounds(bounds, GrOp::HasAABloat::kYes, GrOp::IsZeroArea::kNo);

    // Write the matrix attribs.
    this->writeInstanceData(m.getScaleX(), m.getSkewX(), m.getSkewY(), m.getScaleY());
    this->writeInstanceData(m.getTranslateX(), m.getTranslateY());

    // Convert the radii to [-1, -1, +1, +1] space and write their attribs.
    Sk4f radiiX, radiiY;
    Sk4f::Load2(SkRRectPriv::GetRadiiArray(rrect), &radiiX, &radiiY);
    (radiiX * (2/(r - l))).store(this->appendInstanceData<float>(4));
    (radiiY * (2/(b - t))).store(this->appendInstanceData<float>(4));

    // We will write the color and local rect attribs during finalize().
}

GrProcessorSet::Analysis GrFillRRectOp::finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                                 GrFSAAType fsaaType, GrClampType clampType) {
    SkASSERT(1 == fInstanceCount);

    SkPMColor4f overrideColor;
    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(

            fOriginalColor, GrProcessorAnalysisCoverage::kSingleChannel, clip,
            &GrUserStencilSettings::kUnused, fsaaType, caps, clampType, &overrideColor);

    // Finish writing the instance attribs.
    SkPMColor4f finalColor = analysis.inputColorIsOverridden() ? overrideColor : fOriginalColor;
    if (!SkPMColor4fFitsInBytes(finalColor)) {
        fFlags |= Flags::kWideColor;
        uint32_t halfColor[2];
        SkFloatToHalf_finite_ftz(Sk4f::Load(finalColor.vec())).store(&halfColor);
        this->writeInstanceData(halfColor[0], halfColor[1]);
    } else {
        this->writeInstanceData(finalColor.toBytes_RGBA());
    }

    if (analysis.usesLocalCoords()) {
        this->writeInstanceData(fLocalRect);
        fFlags |= Flags::kHasLocalCoords;
    }
    fInstanceStride = fInstanceData.count();

    return analysis;
}

GrDrawOp::CombineResult GrFillRRectOp::onCombineIfPossible(GrOp* op, const GrCaps&) {
    const auto& that = *op->cast<GrFillRRectOp>();
    if (fFlags != that.fFlags || fProcessors != that.fProcessors ||
        fInstanceData.count() > std::numeric_limits<int>::max() - that.fInstanceData.count()) {
        return CombineResult::kCannotCombine;
    }

    fInstanceData.push_back_n(that.fInstanceData.count(), that.fInstanceData.begin());
    fInstanceCount += that.fInstanceCount;
    SkASSERT(fInstanceStride == that.fInstanceStride);
    return CombineResult::kMerged;
}

void GrFillRRectOp::onPrepare(GrOpFlushState* flushState) {
    if (void* instanceData = flushState->makeVertexSpace(fInstanceStride, fInstanceCount,
                                                         &fInstanceBuffer, &fBaseInstance)) {
        SkASSERT(fInstanceStride * fInstanceCount == fInstanceData.count());
        memcpy(instanceData, fInstanceData.begin(), fInstanceData.count());
    }
}

namespace {

// Our round rect geometry consists of an inset octagon with solid coverage, surrounded by linear
// coverage ramps on the horizontal and vertical edges, and "arc coverage" pieces on the diagonal
// edges. The Vertex struct tells the shader where to place its vertex within a normalized
// ([l, t, r, b] = [-1, -1, +1, +1]) space, and how to calculate coverage. See onEmitCode.
struct Vertex {
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

static constexpr Vertex kVertexData[] = {
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

}

class GrFillRRectOp::Processor : public GrGeometryProcessor {
public:
    Processor(Flags flags)
            : GrGeometryProcessor(kGrFillRRectOp_Processor_ClassID)
            , fFlags(flags) {
        this->setVertexAttributes(kVertexAttribs, 3);
        fInSkew = { "skew", kFloat4_GrVertexAttribType, kFloat4_GrSLType };
        fInTranslate = { "translate", kFloat2_GrVertexAttribType, kFloat2_GrSLType };
        fInRadiiX = { "radii_x", kFloat4_GrVertexAttribType, kFloat4_GrSLType };
        fInRadiiY = { "radii_y", kFloat4_GrVertexAttribType, kFloat4_GrSLType };
        fInColor = MakeColorAttribute("color", (flags & Flags::kWideColor));
        fInLocalRect = {"local_rect", kFloat4_GrVertexAttribType, kFloat4_GrSLType};

        this->setInstanceAttributes(&fInSkew, (flags & Flags::kHasLocalCoords) ? 6 : 5);
        SkASSERT(this->vertexStride() == sizeof(Vertex));
    }

    const char* name() const override { return "GrFillRRectOp::Processor"; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        b->add32(static_cast<uint32_t>(fFlags));
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    static constexpr Attribute kVertexAttribs[] = {
            {"radii_selector", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"corner_and_radius_outsets", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"aa_bloat_and_coverage", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};

    Attribute fInSkew;
    Attribute fInTranslate;
    Attribute fInRadiiX;
    Attribute fInRadiiY;
    Attribute fInColor;
    Attribute fInLocalRect;  // Conditional.

    const Flags fFlags;

    class Impl;
};

constexpr GrPrimitiveProcessor::Attribute GrFillRRectOp::Processor::kVertexAttribs[];

class GrFillRRectOp::Processor::Impl : public GrGLSLGeometryProcessor {
public:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& proc = args.fGP.cast<Processor>();
        bool useHWDerivatives = (proc.fFlags & Flags::kUseHWDerivatives);

        GrGLSLVaryingHandler* varyings = args.fVaryingHandler;
        varyings->emitAttributes(proc);
        varyings->addPassThroughAttribute(proc.fInColor, args.fOutputColor,
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
        if (proc.fFlags & Flags::kHasLocalCoords) {
            v->codeAppend("float2 localcoord = (local_rect.xy * (1 - vertexpos) + "
                                               "local_rect.zw * (1 + vertexpos)) * .5;");
            localCoord.set(kFloat2_GrSLType, "localcoord");
        }
        this->emitTransforms(v, varyings, args.fUniformHandler, localCoord,
                             args.fFPCoordTransformHandler);

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
        f->codeAppendf(    "coverage = half(y);");  // We are a non-arc pixel (i.e., linear coverage).
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
                 FPCoordTransformIter&& transformIter) override {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }
};

GrGLSLPrimitiveProcessor* GrFillRRectOp::Processor::createGLSLInstance(
        const GrShaderCaps&) const {
    return new Impl();
}

void GrFillRRectOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fInstanceBuffer) {
        return;  // Setup failed.
    }

    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);

    sk_sp<const GrBuffer> indexBuffer = flushState->resourceProvider()->findOrMakeStaticBuffer(
            GrGpuBufferType::kIndex, sizeof(kIndexData), kIndexData, gIndexBufferKey);
    if (!indexBuffer) {
        return;
    }

    GR_DEFINE_STATIC_UNIQUE_KEY(gVertexBufferKey);

    sk_sp<const GrBuffer> vertexBuffer = flushState->resourceProvider()->findOrMakeStaticBuffer(
            GrGpuBufferType::kVertex, sizeof(kVertexData), kVertexData, gVertexBufferKey);
    if (!vertexBuffer) {
        return;
    }

    Processor proc(fFlags);
    SkASSERT(proc.instanceStride() == (size_t)fInstanceStride);

    GrPipeline::InitArgs initArgs;
    initArgs.fCaps = &flushState->caps();
    initArgs.fResourceProvider = flushState->resourceProvider();
    initArgs.fDstProxy = flushState->drawOpArgs().fDstProxy;
    auto clip = flushState->detachAppliedClip();
    GrPipeline::FixedDynamicState fixedDynamicState(clip.scissorState().rect());
    GrPipeline pipeline(initArgs, std::move(fProcessors), std::move(clip));

    GrMesh mesh(GrPrimitiveType::kTriangles);
    mesh.setIndexedInstanced(std::move(indexBuffer), SK_ARRAY_COUNT(kIndexData), fInstanceBuffer,
                             fInstanceCount, fBaseInstance, GrPrimitiveRestart::kNo);
    mesh.setVertexData(std::move(vertexBuffer));
    flushState->rtCommandBuffer()->draw(proc, pipeline, &fixedDynamicState, nullptr, &mesh, 1,
                                        this->bounds());
}

// Will the given corner look good if we use HW derivatives?
static bool can_use_hw_derivatives(const Sk2f& devScale, const Sk2f& cornerRadii) {
    Sk2f devRadii = devScale * cornerRadii;
    if (devRadii[1] < devRadii[0]) {
        devRadii = SkNx_shuffle<1,0>(devRadii);
    }
    float minDevRadius = SkTMax(devRadii[0], 1.f);  // Shader clamps radius at a minimum of 1.
    // Is the gradient smooth enough for this corner look ok if we use hardware derivatives?
    // This threshold was arrived at subjevtively on an NVIDIA chip.
    return minDevRadius * minDevRadius * 5 > devRadii[1];
}

static bool can_use_hw_derivatives(const Sk2f& devScale, const SkVector& cornerRadii) {
    return can_use_hw_derivatives(devScale, Sk2f::Load(&cornerRadii));
}

// Will the given round rect look good if we use HW derivatives?
static bool can_use_hw_derivatives(const GrShaderCaps& shaderCaps, const SkMatrix& viewMatrix,
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
            return can_use_hw_derivatives(devScale, rrect.getSimpleRadii());

        case SkRRect::kNinePatch_Type: {
            Sk2f r0 = Sk2f::Load(SkRRectPriv::GetRadiiArray(rrect));
            Sk2f r1 = Sk2f::Load(SkRRectPriv::GetRadiiArray(rrect) + 2);
            Sk2f minRadii = Sk2f::Min(r0, r1);
            Sk2f maxRadii = Sk2f::Max(r0, r1);
            return can_use_hw_derivatives(devScale, Sk2f(minRadii[0], maxRadii[1])) &&
                   can_use_hw_derivatives(devScale, Sk2f(maxRadii[0], minRadii[1]));
        }

        case SkRRect::kComplex_Type: {
            for (int i = 0; i < 4; ++i) {
                auto corner = static_cast<SkRRect::Corner>(i);
                if (!can_use_hw_derivatives(devScale, rrect.radii(corner))) {
                    return false;
                }
            }
            return true;
        }
    }
    SK_ABORT("Unreachable code.");
    return false;  // Add this return to keep GCC happy.
}
