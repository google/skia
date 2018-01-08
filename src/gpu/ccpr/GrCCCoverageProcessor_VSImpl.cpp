/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCCoverageProcessor.h"

#include "GrMesh.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

using Shader = GrCCCoverageProcessor::Shader;

static constexpr int kAttribIdx_X = 0;
static constexpr int kAttribIdx_Y = 1;
static constexpr int kAttribIdx_VertexData = 2;

/**
 * This class and its subclasses implement the coverage processor with vertex shaders.
 */
class GrCCCoverageProcessor::VSImpl : public GrGLSLGeometryProcessor {
protected:
    VSImpl(std::unique_ptr<Shader> shader) : fShader(std::move(shader)) {}

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
        const GrCCCoverageProcessor& proc = args.fGP.cast<GrCCCoverageProcessor>();

        // Vertex shader.
        GrGLSLVertexBuilder* v = args.fVertBuilder;
        int numInputPoints = proc.numInputPoints();

        v->codeAppendf("float%ix2 pts = transpose(float2x%i(%s, %s));",
                       numInputPoints, numInputPoints, proc.getAttrib(kAttribIdx_X).fName,
                       proc.getAttrib(kAttribIdx_Y).fName);

        v->codeAppend ("float area_x2 = determinant(float2x2(pts[0] - pts[1], pts[0] - pts[2]));");
        if (4 == numInputPoints) {
            v->codeAppend ("area_x2 += determinant(float2x2(pts[0] - pts[2], pts[0] - pts[3]));");
        }
        v->codeAppend ("half wind = sign(area_x2);");

        float bloat = kAABloatRadius;
#ifdef SK_DEBUG
        if (proc.debugVisualizationsEnabled()) {
            bloat *= proc.debugBloat();
        }
#endif
        v->defineConstant("bloat", bloat);

        const char* coverage = this->emitVertexPosition(proc, v, gpArgs);
        SkASSERT(kFloat2_GrSLType == gpArgs->fPositionVar.getType());

        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        SkString varyingCode;
        fShader->emitVaryings(varyingHandler, GrGLSLVarying::Scope::kVertToFrag, &varyingCode,
                              gpArgs->fPositionVar.c_str(), coverage, "wind");
        v->codeAppend(varyingCode.c_str());

        varyingHandler->emitAttributes(proc);
        SkASSERT(!args.fFPCoordTransformHandler->nextCoordTransform());

        // Fragment shader.
        fShader->emitFragmentCode(proc, args.fFragBuilder, args.fOutputColor, args.fOutputCoverage);
    }

    virtual const char* emitVertexPosition(const GrCCCoverageProcessor&, GrGLSLVertexBuilder*,
                                           GrGPArgs*) const = 0;

    virtual ~VSImpl() {}

    const std::unique_ptr<Shader> fShader;

    typedef GrGLSLGeometryProcessor INHERITED;
};

/**
 * Vertex data tells the shader how to offset vertices for conservative raster, and how/whether to
 * calculate initial coverage values for edges. See VSHullAndEdgeImpl.
 */
static constexpr int32_t pack_vertex_data(int32_t bloatIdx, int32_t edgeData,
                                          int32_t cornerVertexID, int32_t cornerIdx) {
    return (bloatIdx << 6) | (edgeData << 4) | (cornerVertexID << 2) | cornerIdx;
}

static constexpr int32_t hull_vertex_data(int32_t cornerIdx, int32_t cornerVertexID, int n) {
    return pack_vertex_data((cornerIdx + (2 == cornerVertexID ? 1 : n - 1)) % n, 0, cornerVertexID,
                            cornerIdx);
}

static constexpr int32_t edge_vertex_data(int32_t edgeID, int32_t endptIdx, int32_t endptVertexID,
                                          int n) {
    return pack_vertex_data(0 == endptIdx ? (edgeID + 1) % n : edgeID, (endptIdx << 1) | 1,
                            endptVertexID, 0 == endptIdx ? edgeID : (edgeID + 1) % n);
}

static constexpr int32_t kHull3AndEdgeVertices[] = {
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
};

GR_DECLARE_STATIC_UNIQUE_KEY(gHull3AndEdgeVertexBufferKey);

static constexpr uint16_t kHull3AndEdgeIndices[] =  {
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
};

GR_DECLARE_STATIC_UNIQUE_KEY(gHull3AndEdgeIndexBufferKey);

static constexpr int32_t kHull4Vertices[] = {
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

    // No edges for now (beziers don't use edges).
};

GR_DECLARE_STATIC_UNIQUE_KEY(gHull4VertexBufferKey);

static constexpr uint16_t kHull4Indices[] =  {
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
};

GR_DECLARE_STATIC_UNIQUE_KEY(gHull4IndexBufferKey);

/**
 * Generates a conservative raster hull around a convex polygon. For triangles, we also generate
 * independent conservative rasters around each edge. (See comments for RenderPass)
 */
class VSHullAndEdgeImpl : public GrCCCoverageProcessor::VSImpl {
public:
    VSHullAndEdgeImpl(std::unique_ptr<Shader> shader, int numSides)
            : VSImpl(std::move(shader)), fNumSides(numSides) {}

    const char* emitVertexPosition(const GrCCCoverageProcessor& proc, GrGLSLVertexBuilder* v,
                                   GrGPArgs* gpArgs) const override {
        Shader::GeometryVars vars;
        fShader->emitSetupCode(v, "pts", nullptr, "wind", &vars);

        const char* hullPts = vars.fHullVars.fAlternatePoints;
        if (!hullPts) {
            hullPts = "pts";
        }

        // Reverse all indices if the wind is counter-clockwise: [0, 1, 2] -> [2, 1, 0].
        v->codeAppendf("int clockwise_indices = wind > 0 ? %s : 0x%x - %s;",
                       proc.getAttrib(kAttribIdx_VertexData).fName,
                       ((fNumSides - 1) << 6) | (0xf << 2) | (fNumSides - 1),
                       proc.getAttrib(kAttribIdx_VertexData).fName);

        // Here we generate conservative raster geometry for the input polygon. It is the convex
        // hull of N pixel-size boxes, one centered on each the input points. Each corner has three
        // vertices, where one or two may cause degenerate triangles. The vertex data tells us how
        // to offset each vertex. Triangle edges are also handled here (see kHull3AndEdgeIndices).
        // For more details on conservative raster, see:
        // https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter42.html
        v->codeAppendf("float2 corner = %s[clockwise_indices & 3];", hullPts);
        v->codeAppendf("float2 bloatpoint = %s[clockwise_indices >> 6];", hullPts);
        v->codeAppend ("float2 vertexbloat = float2(bloatpoint.y > corner.y ? -bloat : +bloat, "
                                                   "bloatpoint.x > corner.x ? +bloat : -bloat);");

        v->codeAppendf("if ((1 << 2) == (%s & (3 << 2))) {",
                       proc.getAttrib(kAttribIdx_VertexData).fName);
                           // We are the corner's middle vertex (of 3).
        v->codeAppend (    "vertexbloat = float2(-vertexbloat.y, vertexbloat.x);");
        v->codeAppend ("}");

        v->codeAppendf("if ((2 << 2) == (%s & (3 << 2))) {",
                       proc.getAttrib(kAttribIdx_VertexData).fName);
                           // We are the corner's third vertex (of 3).
        v->codeAppend (    "vertexbloat = -vertexbloat;");
        v->codeAppend ("}");

        v->codeAppend ("float2 vertex = corner + vertexbloat;");
        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");

        if (4 == fNumSides) {
            // We don't generate edges around 4-sided polygons.
            return nullptr; // Known hull vertices don't need an initial coverage value.
        }

        // Find coverage for edge vertices.
        Shader::EmitEdgeDistanceEquation(v, "bloatpoint", "corner",
                                         "float3 edge_distance_equation");
        v->codeAppend ("half coverage = dot(edge_distance_equation.xy, vertex) + "
                                       "edge_distance_equation.z;");
        v->codeAppendf("if (0 == (%s & (1 << 5))) {", proc.getAttrib(kAttribIdx_VertexData).fName);
                           // We are the opposite endpoint. Invert coverage.
        v->codeAppend (    "coverage = -1 - coverage;");
        v->codeAppend ("}");
        v->codeAppendf("if (0 == (%s & (1 << 4))) {", proc.getAttrib(kAttribIdx_VertexData).fName);
                           // We are actually a hull vertex. Hull coverage is +1 all around.
        v->codeAppend (    "coverage = +1;");
        v->codeAppend ("}");

        return "coverage";
    }

private:
    const int fNumSides;
};

static constexpr uint16_t kCornerIndices[] =  {
    // First corner.
    0,  1,  2,
    1,  3,  2,

    // Second corner.
    4,  5,  6,
    5,  7,  6,

    // Third corner.
    8,  9, 10,
    9, 11, 10,
};

GR_DECLARE_STATIC_UNIQUE_KEY(gCornerIndexBufferKey);

/**
 * Generates conservative rasters around corners. (See comments for RenderPass)
 */
class VSCornerImpl : public GrCCCoverageProcessor::VSImpl {
public:
    VSCornerImpl(std::unique_ptr<Shader> shader) : VSImpl(std::move(shader)) {}

    const char* emitVertexPosition(const GrCCCoverageProcessor&, GrGLSLVertexBuilder* v,
                                   GrGPArgs* gpArgs) const override {
        Shader::GeometryVars vars;
        v->codeAppend ("int corner_id = sk_VertexID / 4;");
        fShader->emitSetupCode(v, "pts", "corner_id", "wind", &vars);

        v->codeAppendf("float2 vertex = %s;", vars.fCornerVars.fPoint);
        v->codeAppend ("vertex.x += (0 == (sk_VertexID & 2)) ? -bloat : +bloat;");
        v->codeAppend ("vertex.y += (0 == (sk_VertexID & 1)) ? -bloat : +bloat;");

        gpArgs->fPositionVar.set(kFloat2_GrSLType, "vertex");
        return nullptr; // Corner vertices don't have an initial coverage value.
    }
};

void GrCCCoverageProcessor::initVS(GrResourceProvider* rp) {
    SkASSERT(Impl::kVertexShader == fImpl);

    GrVertexAttribType inputPtsType = RenderPassIsCubic(fRenderPass) ?
                                      kFloat4_GrVertexAttribType : kFloat3_GrVertexAttribType;

    SkASSERT(kAttribIdx_X == this->numAttribs());
    this->addInstanceAttrib("X", inputPtsType);

    SkASSERT(kAttribIdx_Y == this->numAttribs());
    this->addInstanceAttrib("Y", inputPtsType);

    switch (fRenderPass) {
        case RenderPass::kTriangleHulls: {
            GR_DEFINE_STATIC_UNIQUE_KEY(gHull3AndEdgeVertexBufferKey);
            fVertexBuffer = rp->findOrMakeStaticBuffer(kVertex_GrBufferType,
                                                       sizeof(kHull3AndEdgeVertices),
                                                       kHull3AndEdgeVertices,
                                                       gHull3AndEdgeVertexBufferKey);
            GR_DEFINE_STATIC_UNIQUE_KEY(gHull3AndEdgeIndexBufferKey);
            fIndexBuffer = rp->findOrMakeStaticBuffer(kIndex_GrBufferType,
                                                      sizeof(kHull3AndEdgeIndices),
                                                      kHull3AndEdgeIndices,
                                                      gHull3AndEdgeIndexBufferKey);
            SkASSERT(kAttribIdx_VertexData == this->numAttribs());
            this->addVertexAttrib("vertexdata", kInt_GrVertexAttribType);
            break;
        }
        case RenderPass::kQuadraticHulls:
        case RenderPass::kCubicHulls: {
            GR_DEFINE_STATIC_UNIQUE_KEY(gHull4VertexBufferKey);
            fVertexBuffer = rp->findOrMakeStaticBuffer(kVertex_GrBufferType, sizeof(kHull4Vertices),
                                                       kHull4Vertices, gHull4VertexBufferKey);
            GR_DEFINE_STATIC_UNIQUE_KEY(gHull4IndexBufferKey);
            fIndexBuffer = rp->findOrMakeStaticBuffer(kIndex_GrBufferType, sizeof(kHull4Indices),
                                                      kHull4Indices, gHull4IndexBufferKey);
            SkASSERT(kAttribIdx_VertexData == this->numAttribs());
            this->addVertexAttrib("vertexdata", kInt_GrVertexAttribType);
            break;
        }
        case RenderPass::kTriangleEdges:
            SK_ABORT("kTriangleEdges RenderPass is not used by VSImpl.");
            break;
        case RenderPass::kTriangleCorners:
        case RenderPass::kQuadraticCorners:
        case RenderPass::kCubicCorners: {
            GR_DEFINE_STATIC_UNIQUE_KEY(gCornerIndexBufferKey);
            fIndexBuffer = rp->findOrMakeStaticBuffer(kIndex_GrBufferType, sizeof(kCornerIndices),
                                                      kCornerIndices, gCornerIndexBufferKey);
            break;
        }
    }

#ifdef SK_DEBUG
    if (RenderPassIsCubic(fRenderPass)) {
        SkASSERT(offsetof(CubicInstance, fX) == this->getAttrib(kAttribIdx_X).fOffsetInRecord);
        SkASSERT(offsetof(CubicInstance, fY) == this->getAttrib(kAttribIdx_Y).fOffsetInRecord);
        SkASSERT(sizeof(CubicInstance) == this->getInstanceStride());
    } else {
        SkASSERT(offsetof(TriangleInstance, fX) == this->getAttrib(kAttribIdx_X).fOffsetInRecord);
        SkASSERT(offsetof(TriangleInstance, fY) == this->getAttrib(kAttribIdx_Y).fOffsetInRecord);
        SkASSERT(sizeof(TriangleInstance) == this->getInstanceStride());
    }
    if (fVertexBuffer) {
        SkASSERT(sizeof(int32_t) == this->getVertexStride());
    }
#endif
}

static int num_indices_per_instance(GrCCCoverageProcessor::RenderPass pass) {
    switch (pass) {
        using RenderPass = GrCCCoverageProcessor::RenderPass;
        case RenderPass::kTriangleHulls:
            return SK_ARRAY_COUNT(kHull3AndEdgeIndices);
        case RenderPass::kQuadraticHulls:
        case RenderPass::kCubicHulls:
            return SK_ARRAY_COUNT(kHull4Indices);
        case RenderPass::kTriangleEdges:
            SK_ABORT("kTriangleEdges RenderPass is not used by VSImpl.");
            return 0;
        case RenderPass::kTriangleCorners:
            return SK_ARRAY_COUNT(kCornerIndices);
        case RenderPass::kQuadraticCorners:
        case RenderPass::kCubicCorners:
            return SK_ARRAY_COUNT(kCornerIndices) * 2/3;
    }
    SK_ABORT("Invalid RenderPass");
    return 0;
}

void GrCCCoverageProcessor::appendVSMesh(GrBuffer* instanceBuffer, int instanceCount,
                                         int baseInstance, SkTArray<GrMesh>* out) const {
    SkASSERT(Impl::kVertexShader == fImpl);
    GrMesh& mesh = out->emplace_back(GrPrimitiveType::kTriangles);
    mesh.setIndexedInstanced(fIndexBuffer.get(), num_indices_per_instance(fRenderPass),
                             instanceBuffer, instanceCount, baseInstance);
    if (fVertexBuffer) {
        mesh.setVertexData(fVertexBuffer.get(), 0);
    }
}

GrGLSLPrimitiveProcessor* GrCCCoverageProcessor::createVSImpl(std::unique_ptr<Shader> shadr) const {
    switch (fRenderPass) {
        case RenderPass::kTriangleHulls:
            return new VSHullAndEdgeImpl(std::move(shadr), 3);
        case RenderPass::kQuadraticHulls:
        case RenderPass::kCubicHulls:
            return new VSHullAndEdgeImpl(std::move(shadr), 4);
        case RenderPass::kTriangleEdges:
            SK_ABORT("kTriangleEdges RenderPass is not used by VSImpl.");
            return nullptr;
        case RenderPass::kTriangleCorners:
        case RenderPass::kQuadraticCorners:
        case RenderPass::kCubicCorners:
            return new VSCornerImpl(std::move(shadr));
    }
    SK_ABORT("Invalid RenderPass");
    return nullptr;
}
