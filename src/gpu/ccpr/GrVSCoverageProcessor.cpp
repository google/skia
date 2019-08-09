/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrVSCoverageProcessor.h"

#include "src/gpu/GrMesh.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

// This class implements the coverage processor with vertex shaders.
class GrVSCoverageProcessor::Impl : public GrGLSLGeometryProcessor {
public:
    Impl(std::unique_ptr<Shader> shader, int numSides)
            : fShader(std::move(shader)), fNumSides(numSides) {}

private:
    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    const std::unique_ptr<Shader> fShader;
    const int fNumSides;
};

static constexpr int kInstanceAttribIdx_X = 0;  // Transposed X values of all input points.
static constexpr int kInstanceAttribIdx_Y = 1;  // Transposed Y values of all input points.

// Vertex data tells the shader how to offset vertices for conservative raster, as well as how to
// calculate coverage values for corners and edges.
static constexpr int kVertexData_LeftNeighborIdShift = 10;
static constexpr int kVertexData_RightNeighborIdShift = 8;
static constexpr int kVertexData_BloatIdxShift = 6;
static constexpr int kVertexData_InvertNegativeCoverageBit = 1 << 5;
static constexpr int kVertexData_IsCornerBit = 1 << 4;
static constexpr int kVertexData_IsEdgeBit = 1 << 3;
static constexpr int kVertexData_IsHullBit = 1 << 2;

static constexpr int32_t pack_vertex_data(int32_t leftNeighborID, int32_t rightNeighborID,
                                          int32_t bloatIdx, int32_t cornerID,
                                          int32_t extraData = 0) {
    return (leftNeighborID << kVertexData_LeftNeighborIdShift) |
           (rightNeighborID << kVertexData_RightNeighborIdShift) |
           (bloatIdx << kVertexData_BloatIdxShift) |
           cornerID | extraData;
}

static constexpr int32_t hull_vertex_data(int32_t cornerID, int32_t bloatIdx, int n) {
    return pack_vertex_data((cornerID + n - 1) % n, (cornerID + 1) % n, bloatIdx, cornerID,
                            kVertexData_IsHullBit);
}

static constexpr int32_t edge_vertex_data(int32_t edgeID, int32_t endptIdx, int32_t bloatIdx,
                                          int n) {
    return pack_vertex_data(0 == endptIdx ? (edgeID + 1) % n : edgeID,
                            0 == endptIdx ? (edgeID + 1) % n : edgeID,
                            bloatIdx, 0 == endptIdx ? edgeID : (edgeID + 1) % n,
                            kVertexData_IsEdgeBit |
                            (!endptIdx ? kVertexData_InvertNegativeCoverageBit : 0));
}

static constexpr int32_t corner_vertex_data(int32_t leftID, int32_t cornerID, int32_t rightID,
                                            int32_t bloatIdx) {
    return pack_vertex_data(leftID, rightID, bloatIdx, cornerID, kVertexData_IsCornerBit);
}

static constexpr int32_t kTriangleVertices[] = {
    hull_vertex_data(0, 0, 3),
    hull_vertex_data(0, 1, 3),
    hull_vertex_data(0, 2, 3),
    hull_vertex_data(1, 0, 3),
    hull_vertex_data(1, 1, 3),
    hull_vertex_data(1, 2, 3),
    hull_vertex_data(2, 0, 3),
    hull_vertex_data(2, 1, 3),
    hull_vertex_data(2, 2, 3),

    edge_vertex_data(0, 0, 0, 3),
    edge_vertex_data(0, 0, 1, 3),
    edge_vertex_data(0, 0, 2, 3),
    edge_vertex_data(0, 1, 0, 3),
    edge_vertex_data(0, 1, 1, 3),
    edge_vertex_data(0, 1, 2, 3),

    edge_vertex_data(1, 0, 0, 3),
    edge_vertex_data(1, 0, 1, 3),
    edge_vertex_data(1, 0, 2, 3),
    edge_vertex_data(1, 1, 0, 3),
    edge_vertex_data(1, 1, 1, 3),
    edge_vertex_data(1, 1, 2, 3),

    edge_vertex_data(2, 0, 0, 3),
    edge_vertex_data(2, 0, 1, 3),
    edge_vertex_data(2, 0, 2, 3),
    edge_vertex_data(2, 1, 0, 3),
    edge_vertex_data(2, 1, 1, 3),
    edge_vertex_data(2, 1, 2, 3),

    corner_vertex_data(2, 0, 1, 0),
    corner_vertex_data(2, 0, 1, 1),
    corner_vertex_data(2, 0, 1, 2),
    corner_vertex_data(2, 0, 1, 3),

    corner_vertex_data(0, 1, 2, 0),
    corner_vertex_data(0, 1, 2, 1),
    corner_vertex_data(0, 1, 2, 2),
    corner_vertex_data(0, 1, 2, 3),

    corner_vertex_data(1, 2, 0, 0),
    corner_vertex_data(1, 2, 0, 1),
    corner_vertex_data(1, 2, 0, 2),
    corner_vertex_data(1, 2, 0, 3),
};

GR_DECLARE_STATIC_UNIQUE_KEY(gTriangleVertexBufferKey);

static constexpr uint16_t kRestartStrip = 0xffff;

static constexpr uint16_t kTriangleIndicesAsStrips[] =  {
    1, 2, 0, 3, 8, kRestartStrip, // First corner and main body of the hull.
    4, 5, 3, 6, 8, 7, kRestartStrip, // Opposite side and corners of the hull.
    10, 9, 11, 14, 12, 13, kRestartStrip, // First edge.
    16, 15, 17, 20, 18, 19, kRestartStrip, // Second edge.
    22, 21, 23, 26, 24, 25, kRestartStrip, // Third edge.
    28, 27, 29, 30, kRestartStrip, // First corner.
    32, 31, 33, 34, kRestartStrip, // Second corner.
    36, 35, 37, 38 // Third corner.
};

static constexpr uint16_t kTriangleIndicesAsTris[] =  {
    // First corner and main body of the hull.
    1, 2, 0,
    2, 3, 0,
    0, 3, 8, // Main body.

    // Opposite side and corners of the hull.
    4, 5, 3,
    5, 6, 3,
    3, 6, 8,
    6, 7, 8,

    // First edge.
    10,  9, 11,
     9, 14, 11,
    11, 14, 12,
    14, 13, 12,

    // Second edge.
    16, 15, 17,
    15, 20, 17,
    17, 20, 18,
    20, 19, 18,

    // Third edge.
    22, 21, 23,
    21, 26, 23,
    23, 26, 24,
    26, 25, 24,

    // First corner.
    28, 27, 29,
    27, 30, 29,

    // Second corner.
    32, 31, 33,
    31, 34, 33,

    // Third corner.
    36, 35, 37,
    35, 38, 37,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gTriangleIndexBufferKey);

// Curves, including quadratics, are drawn with a four-sided hull.
static constexpr int32_t kCurveVertices[] = {
    hull_vertex_data(0, 0, 4),
    hull_vertex_data(0, 1, 4),
    hull_vertex_data(0, 2, 4),
    hull_vertex_data(1, 0, 4),
    hull_vertex_data(1, 1, 4),
    hull_vertex_data(1, 2, 4),
    hull_vertex_data(2, 0, 4),
    hull_vertex_data(2, 1, 4),
    hull_vertex_data(2, 2, 4),
    hull_vertex_data(3, 0, 4),
    hull_vertex_data(3, 1, 4),
    hull_vertex_data(3, 2, 4),

    corner_vertex_data(3, 0, 1, 0),
    corner_vertex_data(3, 0, 1, 1),
    corner_vertex_data(3, 0, 1, 2),
    corner_vertex_data(3, 0, 1, 3),

    corner_vertex_data(2, 3, 0, 0),
    corner_vertex_data(2, 3, 0, 1),
    corner_vertex_data(2, 3, 0, 2),
    corner_vertex_data(2, 3, 0, 3),
};

GR_DECLARE_STATIC_UNIQUE_KEY(gCurveVertexBufferKey);

static constexpr uint16_t kCurveIndicesAsStrips[] =  {
    1, 0, 2, 11, 3, 5, 4, kRestartStrip, // First half of the hull (split diagonally).
    7, 6, 8, 5, 9, 11, 10, kRestartStrip, // Second half of the hull.
    13, 12, 14, 15, kRestartStrip, // First corner.
    17, 16, 18, 19 // Final corner.
};

static constexpr uint16_t kCurveIndicesAsTris[] =  {
    // First half of the hull (split diagonally).
     1,  0,  2,
     0, 11,  2,
     2, 11,  3,
    11,  5,  3,
     3,  5,  4,

    // Second half of the hull.
    7,  6,  8,
    6,  5,  8,
    8,  5,  9,
    5, 11,  9,
    9, 11, 10,

    // First corner.
    13, 12, 14,
    12, 15, 14,

    // Final corner.
    17, 16, 18,
    16, 19, 18,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gCurveIndexBufferKey);

// Generates a conservative raster hull around a triangle or curve. For triangles we generate
// additional conservative rasters with coverage ramps around the edges and corners.
//
// Triangles are drawn in three steps: (1) Draw a conservative raster of the entire triangle, with a
// coverage of +1. (2) Draw conservative rasters around each edge, with a coverage ramp from -1 to
// 0. These edge coverage values convert jagged conservative raster edges into smooth, antialiased
// ones. (3) Draw conservative rasters (aka pixel-size boxes) around each corner, replacing the
// previous coverage values with ones that ramp to zero in the bloat vertices that fall outside the
// triangle.
//
// Curve shaders handle the opposite edge and corners on their own. For curves we just generate a
// conservative raster here and the shader does the rest.
void GrVSCoverageProcessor::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const GrVSCoverageProcessor& proc = args.fGP.cast<GrVSCoverageProcessor>();
    GrGLSLVertexBuilder* v = args.fVertBuilder;
    int numInputPoints = proc.numInputPoints();

    int inputWidth = (4 == numInputPoints || proc.hasInputWeight()) ? 4 : 3;
    const char* swizzle = (4 == inputWidth) ? "xyzw" : "xyz";
    v->codeAppendf("float%ix2 pts = transpose(float2x%i(%s.%s, %s.%s));", inputWidth, inputWidth,
                   proc.fInputXAndYValues[kInstanceAttribIdx_X].name(), swizzle,
                   proc.fInputXAndYValues[kInstanceAttribIdx_Y].name(), swizzle);

    v->codeAppend ("half wind;");
    Shader::CalcWind(proc, v, "pts", "wind");
    if (PrimitiveType::kWeightedTriangles == proc.fPrimitiveType) {
        SkASSERT(3 == numInputPoints);
        SkASSERT(kFloat4_GrVertexAttribType ==
                 proc.fInputXAndYValues[kInstanceAttribIdx_X].cpuType());
        v->codeAppendf("wind *= half(%s.w);",
                       proc.fInputXAndYValues[kInstanceAttribIdx_X].name());
    }

    float bloat = kAABloatRadius;
#ifdef SK_DEBUG
    if (proc.debugBloatEnabled()) {
        bloat *= proc.debugBloat();
    }
#endif
    v->defineConstant("bloat", bloat);

    const char* hullPts = "pts";
    fShader->emitSetupCode(v, "pts", (4 == fNumSides) ? &hullPts : nullptr);

    // Reverse all indices if the wind is counter-clockwise: [0, 1, 2] -> [2, 1, 0].
    v->codeAppendf("int clockwise_indices = wind > 0 ? %s : 0x%x - %s;",
                   proc.fPerVertexData.name(),
                   ((fNumSides - 1) << kVertexData_LeftNeighborIdShift) |
                   ((fNumSides - 1) << kVertexData_RightNeighborIdShift) |
                   (((1 << kVertexData_RightNeighborIdShift) - 1) ^ 3) |
                   (fNumSides - 1),
                   proc.fPerVertexData.name());

    // Here we generate conservative raster geometry for the input polygon. It is the convex
    // hull of N pixel-size boxes, one centered on each the input points. Each corner has three
    // vertices, where one or two may cause degenerate triangles. The vertex data tells us how
    // to offset each vertex. Triangle edges and corners are also handled here using the same
    // concept. For more details on conservative raster, see:
    // https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter42.html
    v->codeAppendf("float2 corner = %s[clockwise_indices & 3];", hullPts);
    v->codeAppendf("float2 left = %s[clockwise_indices >> %i];",
                   hullPts, kVertexData_LeftNeighborIdShift);
    v->codeAppendf("float2 right = %s[(clockwise_indices >> %i) & 3];",
                   hullPts, kVertexData_RightNeighborIdShift);

    v->codeAppend ("float2 leftbloat = sign(corner - left);");
    v->codeAppend ("leftbloat = float2(0 != leftbloat.y ? leftbloat.y : leftbloat.x, "
                                      "0 != leftbloat.x ? -leftbloat.x : -leftbloat.y);");

    v->codeAppend ("float2 rightbloat = sign(right - corner);");
    v->codeAppend ("rightbloat = float2(0 != rightbloat.y ? rightbloat.y : rightbloat.x, "
                                       "0 != rightbloat.x ? -rightbloat.x : -rightbloat.y);");

    v->codeAppend ("bool2 left_right_notequal = notEqual(leftbloat, rightbloat);");

    v->codeAppend ("float2 bloatdir = leftbloat;");

    v->codeAppend ("float2 leftdir = corner - left;");
    v->codeAppend ("leftdir = (float2(0) != leftdir) ? normalize(leftdir) : float2(1, 0);");

    v->codeAppend ("float2 rightdir = right - corner;");
    v->codeAppend ("rightdir = (float2(0) != rightdir) ? normalize(rightdir) : float2(1, 0);");

    v->codeAppendf("if (0 != (%s & %i)) {",  // Are we a corner?
                   proc.fPerVertexData.name(), kVertexData_IsCornerBit);

                       // In corner boxes, all 4 coverage values will not map linearly.
                       // Therefore it is important to align the box so its diagonal shared
                       // edge points out of the triangle, in the direction that ramps to 0.
    v->codeAppend (    "bloatdir = float2(leftdir.x > rightdir.x ? +1 : -1, "
                                         "leftdir.y > rightdir.y ? +1 : -1);");

                       // For corner boxes, we hack left_right_notequal to always true. This
                       // in turn causes the upcoming code to always rotate, generating all
                       // 4 vertices of the corner box.
    v->codeAppendf(    "left_right_notequal = bool2(true);");
    v->codeAppend ("}");

    // At each corner of the polygon, our hull will have either 1, 2, or 3 vertices (or 4 if
    // it's a corner box). We begin with this corner's first raster vertex (leftbloat), then
    // continue rotating 90 degrees clockwise until we reach the desired raster vertex for this
    // invocation. Corners with less than 3 corresponding raster vertices will result in
    // redundant vertices and degenerate triangles.
    v->codeAppendf("int bloatidx = (%s >> %i) & 3;", proc.fPerVertexData.name(),
                   kVertexData_BloatIdxShift);
    v->codeAppend ("switch (bloatidx) {");
    v->codeAppend (    "case 3:");
                            // Only corners will have bloatidx=3, and corners always rotate.
    v->codeAppend (        "bloatdir = float2(-bloatdir.y, +bloatdir.x);"); // 90 deg CW.
                           // fallthru.
    v->codeAppend (    "case 2:");
    v->codeAppendf(        "if (all(left_right_notequal)) {");
    v->codeAppend (            "bloatdir = float2(-bloatdir.y, +bloatdir.x);"); // 90 deg CW.
    v->codeAppend (        "}");
                           // fallthru.
    v->codeAppend (    "case 1:");
    v->codeAppendf(        "if (any(left_right_notequal)) {");
    v->codeAppend (            "bloatdir = float2(-bloatdir.y, +bloatdir.x);"); // 90 deg CW.
    v->codeAppend (        "}");
                           // fallthru.
    v->codeAppend ("}");

    v->codeAppend ("float2 vertexpos = fma(bloatdir, float2(bloat), corner);");
    gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertexpos");

    // Hulls have a coverage of +1 all around.
    v->codeAppend ("half coverage = +1;");

    if (3 == fNumSides) {
        v->codeAppend ("half left_coverage; {");
        Shader::CalcEdgeCoverageAtBloatVertex(v, "left", "corner", "bloatdir", "left_coverage");
        v->codeAppend ("}");

        v->codeAppend ("half right_coverage; {");
        Shader::CalcEdgeCoverageAtBloatVertex(v, "corner", "right", "bloatdir", "right_coverage");
        v->codeAppend ("}");

        v->codeAppendf("if (0 != (%s & %i)) {",  // Are we an edge?
                       proc.fPerVertexData.name(), kVertexData_IsEdgeBit);
        v->codeAppend (    "coverage = left_coverage;");
        v->codeAppend ("}");

        v->codeAppendf("if (0 != (%s & %i)) {",  // Invert coverage?
                       proc.fPerVertexData.name(),
                       kVertexData_InvertNegativeCoverageBit);
        v->codeAppend (    "coverage = -1 - coverage;");
        v->codeAppend ("}");
    } else if (!fShader->calculatesOwnEdgeCoverage()) {
        // Determine the amount of coverage to subtract out for the flat edge of the curve.
        v->codeAppendf("float2 p0 = pts[0], p1 = pts[%i];", numInputPoints - 1);
        v->codeAppendf("float2 n = float2(p0.y - p1.y, p1.x - p0.x);");
        v->codeAppend ("float nwidth = bloat*2 * (abs(n.x) + abs(n.y));");
        // When nwidth=0, wind must also be 0 (and coverage * wind = 0). So it doesn't matter
        // what we come up with here as long as it isn't NaN or Inf.
        v->codeAppend ("float d = dot(p0 - vertexpos, n);");
        v->codeAppend ("d /= (0 != nwidth) ? nwidth : 1;");
        v->codeAppend ("coverage = half(d) - .5*sign(wind);");
    }

    // Non-corner geometry should have zero effect from corner coverage.
    v->codeAppend ("half2 corner_coverage = half2(0);");

    v->codeAppendf("if (0 != (%s & %i)) {",  // Are we a corner?
                   proc.fPerVertexData.name(), kVertexData_IsCornerBit);
                       // Erase what the previous geometry wrote.
    v->codeAppend (    "wind = -wind;");
    if (3 == fNumSides) {
        v->codeAppend ("coverage = 1 + left_coverage + right_coverage;");
    } else if (!fShader->calculatesOwnEdgeCoverage()) {
        v->codeAppend ("coverage = -coverage;");
    }

                       // Corner boxes require attenuated coverage.
    v->codeAppend (    "half attenuation; {");
    Shader::CalcCornerAttenuation(v, "leftdir", "rightdir", "attenuation");
    v->codeAppend (    "}");

                       // Attenuate corner coverage towards the outermost vertex (where bloatidx=0).
                       // This is all that curves need: At each vertex of the corner box, the curve
                       // Shader will calculate the curve's local coverage value, interpolate it
                       // alongside our attenuation parameter, and multiply the two together for a
                       // final coverage value.
    v->codeAppend (    "corner_coverage = (0 == bloatidx) ? half2(0, attenuation) : half2(-1,+1);");

    if (3 == fNumSides) {
                       // For triangles we also provide the actual coverage values at each vertex of
                       // the corner box.
        v->codeAppend ("if (1 == bloatidx || 2 == bloatidx) {");
        v->codeAppend (    "corner_coverage.x -= right_coverage;");
        v->codeAppend ("}");
        v->codeAppend ("if (bloatidx >= 2) {");
        v->codeAppend (    "corner_coverage.x -= left_coverage;");
        v->codeAppend ("}");
    }
    v->codeAppend ("}");

    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    v->codeAppend ("coverage *= wind;");
    v->codeAppend ("corner_coverage.x *= wind;");
    fShader->emitVaryings(varyingHandler, GrGLSLVarying::Scope::kVertToFrag, &AccessCodeString(v),
                          "vertexpos", "coverage", "corner_coverage", "wind");

    varyingHandler->emitAttributes(proc);
    SkASSERT(!args.fFPCoordTransformHandler->nextCoordTransform());

    // Fragment shader.
    GrGLSLFPFragmentBuilder* f = args.fFragBuilder;
    f->codeAppendf("half coverage;");
    fShader->emitFragmentCoverageCode(f, "coverage");
    f->codeAppendf("%s = half4(coverage);", args.fOutputColor);
    f->codeAppendf("%s = half4(1);", args.fOutputCoverage);
}

void GrVSCoverageProcessor::reset(PrimitiveType primitiveType, GrResourceProvider* rp) {
    const GrCaps& caps = *rp->caps();

    fPrimitiveType = primitiveType;
    switch (fPrimitiveType) {
        case PrimitiveType::kTriangles:
        case PrimitiveType::kWeightedTriangles: {
            GR_DEFINE_STATIC_UNIQUE_KEY(gTriangleVertexBufferKey);
            fVertexBuffer = rp->findOrMakeStaticBuffer(
                    GrGpuBufferType::kVertex, sizeof(kTriangleVertices), kTriangleVertices,
                    gTriangleVertexBufferKey);
            GR_DEFINE_STATIC_UNIQUE_KEY(gTriangleIndexBufferKey);
            if (caps.usePrimitiveRestart()) {
                fIndexBuffer = rp->findOrMakeStaticBuffer(
                        GrGpuBufferType::kIndex, sizeof(kTriangleIndicesAsStrips),
                        kTriangleIndicesAsStrips, gTriangleIndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kTriangleIndicesAsStrips);
            } else {
                fIndexBuffer = rp->findOrMakeStaticBuffer(
                        GrGpuBufferType::kIndex, sizeof(kTriangleIndicesAsTris),
                        kTriangleIndicesAsTris, gTriangleIndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kTriangleIndicesAsTris);
            }
            break;
        }

        case PrimitiveType::kQuadratics:
        case PrimitiveType::kCubics:
        case PrimitiveType::kConics: {
            GR_DEFINE_STATIC_UNIQUE_KEY(gCurveVertexBufferKey);
            fVertexBuffer = rp->findOrMakeStaticBuffer(
                    GrGpuBufferType::kVertex, sizeof(kCurveVertices), kCurveVertices,
                    gCurveVertexBufferKey);
            GR_DEFINE_STATIC_UNIQUE_KEY(gCurveIndexBufferKey);
            if (caps.usePrimitiveRestart()) {
                fIndexBuffer = rp->findOrMakeStaticBuffer(
                        GrGpuBufferType::kIndex, sizeof(kCurveIndicesAsStrips),
                        kCurveIndicesAsStrips, gCurveIndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kCurveIndicesAsStrips);
            } else {
                fIndexBuffer = rp->findOrMakeStaticBuffer(
                        GrGpuBufferType::kIndex, sizeof(kCurveIndicesAsTris), kCurveIndicesAsTris,
                        gCurveIndexBufferKey);
                fNumIndicesPerInstance = SK_ARRAY_COUNT(kCurveIndicesAsTris);
            }
            break;
        }
    }

    GrVertexAttribType xyAttribType;
    GrSLType xySLType;
    if (4 == this->numInputPoints() || this->hasInputWeight()) {
        GR_STATIC_ASSERT(offsetof(QuadPointInstance, fX) == 0);
        GR_STATIC_ASSERT(sizeof(QuadPointInstance::fX) ==
                         GrVertexAttribTypeSize(kFloat4_GrVertexAttribType));
        GR_STATIC_ASSERT(sizeof(QuadPointInstance::fY) ==
                         GrVertexAttribTypeSize(kFloat4_GrVertexAttribType));
        xyAttribType = kFloat4_GrVertexAttribType;
        xySLType = kFloat4_GrSLType;
    } else {
        GR_STATIC_ASSERT(sizeof(TriPointInstance) ==
                         2 * GrVertexAttribTypeSize(kFloat3_GrVertexAttribType));
        xyAttribType = kFloat3_GrVertexAttribType;
        xySLType = kFloat3_GrSLType;
    }
    fInputXAndYValues[kInstanceAttribIdx_X] = {"X", xyAttribType, xySLType};
    fInputXAndYValues[kInstanceAttribIdx_Y] = {"Y", xyAttribType, xySLType};
    this->setInstanceAttributes(fInputXAndYValues, 2);
    fPerVertexData = {"vertexdata", kInt_GrVertexAttribType, kInt_GrSLType};
    this->setVertexAttributes(&fPerVertexData, 1);

    if (caps.usePrimitiveRestart()) {
        fTriangleType = GrPrimitiveType::kTriangleStrip;
    } else {
        fTriangleType = GrPrimitiveType::kTriangles;
    }
}

void GrVSCoverageProcessor::appendMesh(sk_sp<const GrGpuBuffer> instanceBuffer, int instanceCount,
                                       int baseInstance, SkTArray<GrMesh>* out) const {
    GrMesh& mesh = out->emplace_back(fTriangleType);
    auto primitiveRestart = GrPrimitiveRestart(GrPrimitiveType::kTriangleStrip == fTriangleType);
    mesh.setIndexedInstanced(fIndexBuffer, fNumIndicesPerInstance, std::move(instanceBuffer),
                             instanceCount, baseInstance, primitiveRestart);
    mesh.setVertexData(fVertexBuffer, 0);
}

GrGLSLPrimitiveProcessor* GrVSCoverageProcessor::onCreateGLSLInstance(
        std::unique_ptr<Shader> shader) const {
    switch (fPrimitiveType) {
        case PrimitiveType::kTriangles:
        case PrimitiveType::kWeightedTriangles:
            return new Impl(std::move(shader), 3);
        case PrimitiveType::kQuadratics:
        case PrimitiveType::kCubics:
        case PrimitiveType::kConics:
            return new Impl(std::move(shader), 4);
    }
    SK_ABORT("Invalid PrimitiveType");
}
