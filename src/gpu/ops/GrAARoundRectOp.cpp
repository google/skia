/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAARoundRectOp.h"

#include "GrCaps.h"
#include "GrContextPriv.h"
#include "GrGpuCommandBuffer.h"
#include "GrMemoryPool.h"
#include "SkRRectPriv.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

// Hardware derivatives are not always accurate enough for highly elliptical corners. This method
// checks to make sure the corners will still all look good if we use HW derivatives.
static bool can_use_hw_derivatives(const GrShaderCaps&, const SkMatrix&, const SkRRect&);

std::unique_ptr<GrAARoundRectOp> GrAARoundRectOp::Make(
        GrContext* ctx, const SkMatrix& m, const SkRRect& rrect, const GrCaps& caps,
        GrPaint&& paint) {
    if (!caps.instanceAttribSupport() || !caps.shaderCaps()->integerSupport()) {
        return nullptr;
    }

    // TODO: Support perspective in a follow-on CL. This shouldn't be difficult, since we already
    // use HW derivaties. The only trick will be adjusting the AA outset to account for perspective.
    // (i.e., outset = 0.5 * z.)
    if (m.hasPerspective()) {
        return nullptr;
    }

    GrOpMemoryPool* pool = ctx->contextPriv().opMemoryPool();
    return pool->allocate<GrAARoundRectOp>(*caps.shaderCaps(), m, rrect, std::move(paint));
}

GrAARoundRectOp::GrAARoundRectOp(const GrShaderCaps& shaderCaps, const SkMatrix& viewMatrix,
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

    // Since m is a 2x3 matrix that maps the rect [-1, -1, +1, +1] into the shape's
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
    Sk4f::Load2(SkRRectPriv::AccessRadiusBuffer(rrect), &radiiX, &radiiY);
    (radiiX * (2/(r - l))).store(this->appendInstanceData<float>(4));
    (radiiY * (2/(b - t))).store(this->appendInstanceData<float>(4));

    // We will write the color and local rect attribs during finalize().
}

GrDrawOp::RequiresDstTexture GrAARoundRectOp::finalize(const GrCaps& caps,
                                                       const GrAppliedClip* clip) {
    SkASSERT(1 == fInstanceCount);

    SkPMColor4f overrideColor;
    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(
            fOriginalColor, GrProcessorAnalysisCoverage::kSingleChannel, clip, false, caps,
            &overrideColor);

    // Finish writing the instance attribs.
    this->writeInstanceData(
            (analysis.inputColorIsOverridden() ? overrideColor : fOriginalColor).toBytes_RGBA());
    if (analysis.usesLocalCoords()) {
        this->writeInstanceData(fLocalRect);
        fFlags |= Flags::kHasLocalCoords;
    }
    fInstanceStride = fInstanceData.count();

    return RequiresDstTexture(analysis.requiresDstTexture());
}

GrDrawOp::CombineResult GrAARoundRectOp::onCombineIfPossible(GrOp* op, const GrCaps&) {
    const auto& that = *op->cast<GrAARoundRectOp>();
    if (fFlags != that.fFlags || fProcessors != that.fProcessors ||
        fInstanceData.count() > std::numeric_limits<int>::max() - that.fInstanceData.count()) {
        return CombineResult::kCannotCombine;
    }

    fInstanceData.push_back_n(that.fInstanceData.count(), that.fInstanceData.begin());
    fInstanceCount += that.fInstanceCount;
    SkASSERT(fInstanceStride == that.fInstanceStride);
    return CombineResult::kMerged;
}

void GrAARoundRectOp::onPrepare(GrOpFlushState* flushState) {
    if (void* instanceData = flushState->makeVertexSpace(fInstanceStride, fInstanceCount,
                                                         &fInstanceBuffer, &fBaseInstance)) {
        SkASSERT(fInstanceStride * fInstanceCount == fInstanceData.count());
        memcpy(instanceData, fInstanceData.begin(), fInstanceData.count());
    }
}

namespace {

// We draw round rects with 8 sections, fanned out from the center: 4 corners and 4 edges.
//
// Corner pieces have 4 vertices: center -> (corner - y_radius) -> (corner - x_radius) -> corner.
//
// Edge pieces are triangles that fill in the rest of the space between the corner segments.
struct Vertex {
    int16_t fCornerID;  // Which corner should this vertex offset from?
    int16_t fVertexID;  // Vertex ID within the section.  (Modulo 4.)
                        //   vertex-id < 4: Indicates we are a corner segment.
                        //   4 <= vertex-id < 8: Indicates we are an x-edge segment.
                        //   vertex-id >= 8: Indicates we are a y-edge segment.
};

static constexpr Vertex kVertexData[] = {
        {0,        0},  // top-left corner
        {0,        2},
        {0,        1},
        {0,        3},

        {1,        0},  // top-right corner
        {1,        1},
        {1,        2},
        {1,        3},

        {2,        0},  // bottom-left corner
        {2,        1},
        {2,        2},
        {2,        3},

        {3,        0},  // bottom-right corner
        {3,        2},
        {3,        1},
        {3,        3},

        {0,    4 + 0},  // left edge
        {2,    4 + 2},
        {0,    4 + 2},

        {0,    8 + 0},  // top edge
        {0,    8 + 1},
        {1,    8 + 1},

        {0,    4 + 0},  // right edge
        {1,    4 + 2},
        {3,    4 + 2},

        {0,    8 + 0},  // bottom edge
        {3,    8 + 1},
        {2,    8 + 1},
};

GR_DECLARE_STATIC_UNIQUE_KEY(gVertexBufferKey);

static constexpr uint16_t kIndexData[] = {
     0,  1,  2,  1,  3,  2,  // top-left corner
     4,  5,  6,  5,  7,  6,  // top-right corner
     8,  9, 10,  9, 11, 10,  // bottom-left corner
    12, 13, 14, 13, 15, 14,  // bottom-right corner

    16, 17, 18,  // left edge
    19, 20, 21,  // top edge
    22, 23, 24,  // right edge
    25, 26, 27,  // bottom edge
};

GR_DECLARE_STATIC_UNIQUE_KEY(gIndexBufferKey);

}

class GrAARoundRectOp::RoundRectProc : public GrGeometryProcessor {
public:
    RoundRectProc(Flags flags)
            : GrGeometryProcessor(kAARoundRectProcessor_ClassID)
            , fFlags(flags) {
        this->setVertexAttributes(&kVertexAttrib, 1);
        this->setInstanceAttributes(kInstanceAttribs, (flags & Flags::kHasLocalCoords) ? 6 : 5);
        SkASSERT(this->vertexStride() == sizeof(Vertex));
    }

    const char* name() const override { return "GrAARoundRectOp::RoundRectProc"; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        b->add32(static_cast<uint32_t>(fFlags));
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override;

private:
    static constexpr Attribute kVertexAttrib =
            {"corner_and_vertex_ids", kShort2_GrVertexAttribType, kShort2_GrSLType};

    static constexpr Attribute kInstanceAttribs[] = {
            {"skew", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"translate", kFloat2_GrVertexAttribType, kFloat2_GrSLType},
            {"radii_x", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"radii_y", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"color", kUByte4_norm_GrVertexAttribType, kHalf4_GrSLType},
            {"local_rect", kFloat4_GrVertexAttribType, kFloat4_GrSLType}};  // Conditional.

    static constexpr int kColorAttribIdx = 4;

    const Flags fFlags;

    class Impl;
};

constexpr GrPrimitiveProcessor::Attribute GrAARoundRectOp::RoundRectProc::kVertexAttrib;
constexpr GrPrimitiveProcessor::Attribute GrAARoundRectOp::RoundRectProc::kInstanceAttribs[];

class GrAARoundRectOp::RoundRectProc::Impl : public GrGLSLGeometryProcessor {
public:
    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
        const auto& proc = args.fGP.cast<RoundRectProc>();
        bool useHWDerivatives = (proc.fFlags & Flags::kUseHWDerivatives);

        GrGLSLVaryingHandler* varyings = args.fVaryingHandler;
        varyings->emitAttributes(proc);
        varyings->addPassThroughAttribute(proc.kInstanceAttribs[kColorAttribIdx], args.fOutputColor,
                                          GrGLSLVaryingHandler::Interpolation::kCanBeFlat);

        // Emit the vertex shader.
        GrGLSLVertexBuilder* v = args.fVertBuilder;

        // Identify the offset to our corner.
        // The view matrix maps from a normalized space where [l, t, r, b] == [-1, -1, +1, +1].
        v->codeAppend("short corner_id = corner_and_vertex_ids[0];");
        v->codeAppend("float2 corner = float2((0 == (corner_id & 1)) ? -1 : +1, "
                                             "(0 == (corner_id >> 1)) ? -1 : +1);");

        // Find our radii.
        v->codeAppend("float2 pixellength = inversesqrt("
                              "float2(dot(skew.xz, skew.xz), dot(skew.yw, skew.yw)));");
        v->codeAppend("float2 minradii = pixellength;");  // Min radius == 1px.
        v->codeAppend("float2 maxradii = 2 - minradii;");
        // SkRRect stores radii in "fan" order, whereas corner_id is in "strip" order. The following
        // bit twiddle resolves this disparity by swapping the values 2 and 3.
        v->codeAppend("short radius_id = corner_id ^ (corner_id >> 1);");
        v->codeAppend("float2 radii = float2(radii_x[radius_id], radii_y[radius_id]);");
        v->codeAppend("radii = clamp(radii, minradii, maxradii);");
        v->codeAppend("radii *= -corner;");

        // Find the vertex position based on our vertex id within the segment.
        v->codeAppend("short vertex_id = corner_and_vertex_ids[1] & 3;");
        v->codeAppend("float2 vertexpos = (0 == vertex_id) ? float2(0) : corner;");
        v->codeAppend("if (1 == vertex_id)");
        v->codeAppend(    "vertexpos.x += radii.x;");
        v->codeAppend("else if (2 == vertex_id)");
        v->codeAppend(    "vertexpos.y += radii.y;");

        // Bloat out for AA.
        v->codeAppend ("float4 normalized_axis_dirs = skew * pixellength.xyxy;");
        v->codeAppend ("float2 outsetwidths = (abs(normalized_axis_dirs.xy) + "
                                              "abs(normalized_axis_dirs.zw)) * .5;");
        v->codeAppend ("float2 bloat = outsetwidths * pixellength * corner;");
        v->codeAppend("if (vertexpos.x == corner.x)");
        v->codeAppend(    "vertexpos.x += bloat.x;");
        v->codeAppend("if (vertexpos.y == corner.y)");
        v->codeAppend(    "vertexpos.y += bloat.y;");

        // Emit transforms.
        GrShaderVar localCoord("", kFloat2_GrSLType);
        if (proc.fFlags & Flags::kHasLocalCoords) {
            v->codeAppendf("float2 localcoord = (local_rect.xy * (1 - vertexpos) + "
                                                "local_rect.zw * (1 + vertexpos)) * .5;");
            localCoord.set(kFloat2_GrSLType, "localcoord");
        }
        this->emitTransforms(v, varyings, args.fUniformHandler, localCoord,
                             args.fFPCoordTransformHandler);

        // Setup the implicit arc equation for our corner/edge (i.e, x^2 + y^2 == 1).
        GrGLSLVarying arcCoord(kFloat2_GrSLType);
        varyings->addVarying("arc_coord", &arcCoord);

        GrGLSLVarying derivatives(kFloat4_GrSLType);  // MSL does not support varying float2x2.
        if (!useHWDerivatives) {
            varyings->addVarying("derivatives", &derivatives,
                                 GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
        }

        v->codeAppendf("float2x2 skewmatrix = float2x2(skew.xy, skew.zw);");
        if (!useHWDerivatives) {
            if (!args.fShaderCaps->matrixInverseFunctionIsMissing()) {
                v->codeAppendf("float2x2 derivatives = inverse(skewmatrix);");
            } else {
                v->codeAppendf("float d = determinant(skewmatrix);");
                v->codeAppendf("float4 inverseskew = skew.wyzx / float4(+d,-d,-d,+d);");
                v->codeAppendf("float2x2 derivatives = float2x2(inverseskew.xy, inverseskew.zw);");
            }
        }
        v->codeAppend("short segtype = corner_and_vertex_ids[1] >> 2;");
        v->codeAppend("if (0 == segtype) {");
                           // We are a corner segment. Emit our corner's arc equation.
        v->codeAppendf(    "%s.xy = (corner + radii - vertexpos) / radii;", arcCoord.vsOut());
        if (!useHWDerivatives) {
            v->codeAppendf("%s.xy = derivatives[0] * -2/radii.x;", derivatives.vsOut());
            v->codeAppendf("%s.zw = derivatives[1] * -2/radii.y;", derivatives.vsOut());
        }
        v->codeAppend("} else {");
                           // We are an edge segment. Emit a degenerate arc equation that is
                           // actually just a flat line at our corresponding edge. (x^2 + 0 == 1.)
        v->codeAppendf(    "short axis = segtype - 1;");
        v->codeAppendf(    "%s.xy = (float2(vertexpos[axis] * corner[axis] * .5 + .5, 0));",
                           arcCoord.vsOut());
        if (!useHWDerivatives) {
            v->codeAppendf("%s.xy = derivatives[axis];", derivatives.vsOut());
            v->codeAppendf("%s.zw = float2(0);", derivatives.vsOut());
        }
        v->codeAppend("}");

        // Transform to device coords.
        v->codeAppend("float2 devcoord = vertexpos * skewmatrix + translate;");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "devcoord");

        // Emit the fragment shader.
        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

        // Clamp the arc coord to our own corner's quadrant.
        f->codeAppendf("float2 coord = max(%s, 0);", arcCoord.fsIn());
        f->codeAppendf("float fn = dot(coord, coord) - 1;");  // x^2 + y^2 == 1
        if (useHWDerivatives) {
            f->codeAppendf("float fnwidth = fwidth(fn);");
        } else {
            // The gradient is not smooth enough for HW derivatives to look good. Evaluate the
            // derivatives symbolically. (And we can't just interpolate "grad" because "coord"
            // became nonlinear when we clamped it above zero.)
            f->codeAppendf("float2 grad = float2x2(%s.xy, %s.zw) * coord;",
                           derivatives.fsIn(), derivatives.fsIn());
            f->codeAppendf("float fnwidth = abs(grad.x) + abs(grad.y);");
        }
        f->codeAppendf("fnwidth += 1e-15;");  // Guard against division by zero.
        f->codeAppendf("%s = half4(clamp(.5 - fn/fnwidth, 0, 1));", args.fOutputCoverage);
    }

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) override {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }
};

GrGLSLPrimitiveProcessor* GrAARoundRectOp::RoundRectProc::createGLSLInstance(
        const GrShaderCaps&) const {
    return new Impl();
}

void GrAARoundRectOp::onExecute(GrOpFlushState* flushState) {
    if (!fInstanceBuffer) {
        return;  // Setup failed.
    }

    GR_DEFINE_STATIC_UNIQUE_KEY(gIndexBufferKey);

    sk_sp<const GrBuffer> indexBuffer =
            flushState->resourceProvider()->findOrMakeStaticBuffer(
                    kIndex_GrBufferType, sizeof(kIndexData), kIndexData, gIndexBufferKey);
    if (!indexBuffer) {
        return;
    }

    GR_DEFINE_STATIC_UNIQUE_KEY(gVertexBufferKey);

    sk_sp<const GrBuffer> vertexBuffer =
            flushState->resourceProvider()->findOrMakeStaticBuffer(
                    kVertex_GrBufferType, sizeof(kVertexData), kVertexData, gVertexBufferKey);
    if (!vertexBuffer) {
        return;
    }

    RoundRectProc proc(fFlags);
    SkASSERT(proc.instanceStride() == (size_t)fInstanceStride);

    GrPipeline::InitArgs initArgs;
    initArgs.fProxy = flushState->drawOpArgs().fProxy;
    initArgs.fCaps = &flushState->caps();
    initArgs.fResourceProvider = flushState->resourceProvider();
    initArgs.fDstProxy = flushState->drawOpArgs().fDstProxy;
    GrPipeline pipeline(initArgs, std::move(fProcessors), flushState->detachAppliedClip());

    GrMesh mesh(GrPrimitiveType::kTriangles);
    mesh.setIndexedInstanced(indexBuffer.get(), SK_ARRAY_COUNT(kIndexData), fInstanceBuffer,
                             fInstanceCount, fBaseInstance, GrPrimitiveRestart::kNo);
    mesh.setVertexData(vertexBuffer.get());
    flushState->rtCommandBuffer()->draw(proc, pipeline, nullptr, nullptr, &mesh, 1, this->bounds());
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
            Sk2f r0 = Sk2f::Load(SkRRectPriv::AccessRadiusBuffer(rrect));
            Sk2f r1 = Sk2f::Load(SkRRectPriv::AccessRadiusBuffer(rrect) + 2);
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
