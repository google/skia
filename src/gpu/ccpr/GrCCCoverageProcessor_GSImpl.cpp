/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCCoverageProcessor.h"

#include "GrMesh.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

using InputType = GrGLSLGeometryBuilder::InputType;
using OutputType = GrGLSLGeometryBuilder::OutputType;
using Shader = GrCCCoverageProcessor::Shader;

/**
 * This class and its subclasses implement the coverage processor with geometry shaders.
 */
class GrCCCoverageProcessor::GSImpl : public GrGLSLGeometryProcessor {
protected:
    GSImpl(std::unique_ptr<Shader> shader) : fShader(std::move(shader)) {}

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
        const GrCCCoverageProcessor& proc = args.fGP.cast<GrCCCoverageProcessor>();

        // The vertex shader simply forwards transposed x or y values to the geometry shader.
        SkASSERT(1 == proc.numAttribs());
        gpArgs->fPositionVar.set(4 == proc.numInputPoints() ? kFloat4_GrSLType : kFloat3_GrSLType,
                                 proc.getAttrib(0).fName);

        // Geometry shader.
        GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
        this->emitGeometryShader(proc, varyingHandler, args.fGeomBuilder, args.fRTAdjustName);
        varyingHandler->emitAttributes(proc);
        varyingHandler->setNoPerspective();
        SkASSERT(!args.fFPCoordTransformHandler->nextCoordTransform());

        // Fragment shader.
        fShader->emitFragmentCode(proc, args.fFragBuilder, args.fOutputColor, args.fOutputCoverage);
    }

    void emitGeometryShader(const GrCCCoverageProcessor& proc,
                            GrGLSLVaryingHandler* varyingHandler, GrGLSLGeometryBuilder* g,
                            const char* rtAdjust) const {
        int numInputPoints = proc.numInputPoints();
        SkASSERT(3 == numInputPoints || 4 == numInputPoints);

        const char* posValues = (4 == numInputPoints) ? "sk_Position" : "sk_Position.xyz";
        g->codeAppendf("float%ix2 pts = transpose(float2x%i(sk_in[0].%s, sk_in[1].%s));",
                       numInputPoints, numInputPoints, posValues, posValues);

        GrShaderVar wind("wind", kHalf_GrSLType);
        g->declareGlobal(wind);
        g->codeAppend ("float area_x2 = determinant(float2x2(pts[0] - pts[1], pts[0] - pts[2]));");
        if (4 == numInputPoints) {
            g->codeAppend ("area_x2 += determinant(float2x2(pts[0] - pts[2], pts[0] - pts[3]));");
        }
        g->codeAppendf("%s = sign(area_x2);", wind.c_str());

        SkString emitVertexFn;
        SkSTArray<2, GrShaderVar> emitArgs;
        const char* position = emitArgs.emplace_back("position", kFloat2_GrSLType).c_str();
        const char* coverage = nullptr;
        if (RenderPass::kTriangleEdges == proc.fRenderPass) {
            coverage = emitArgs.emplace_back("coverage", kHalf_GrSLType).c_str();
        }
        g->emitFunction(kVoid_GrSLType, "emitVertex", emitArgs.count(), emitArgs.begin(), [&]() {
            SkString fnBody;
            fShader->emitVaryings(varyingHandler, GrGLSLVarying::Scope::kGeoToFrag, &fnBody,
                                  position, coverage, wind.c_str());
            g->emitVertex(&fnBody, position, rtAdjust);
            return fnBody;
        }().c_str(), &emitVertexFn);

        float bloat = kAABloatRadius;
#ifdef SK_DEBUG
        if (proc.debugVisualizationsEnabled()) {
            bloat *= proc.debugBloat();
        }
#endif
        g->defineConstant("bloat", bloat);

        this->onEmitGeometryShader(g, wind, emitVertexFn.c_str());
    }

    virtual void onEmitGeometryShader(GrGLSLGeometryBuilder*, const GrShaderVar& wind,
                                      const char* emitVertexFn) const = 0;

    virtual ~GSImpl() {}

    const std::unique_ptr<Shader> fShader;

    typedef GrGLSLGeometryProcessor INHERITED;
};

/**
 * Generates a conservative raster hull around a triangle. (See comments for RenderPass)
 */
class GSHull3Impl : public GrCCCoverageProcessor::GSImpl {
public:
    GSHull3Impl(std::unique_ptr<Shader> shader) : GSImpl(std::move(shader)) {}

    void onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                              const char* emitVertexFn) const override {
        Shader::GeometryVars vars;
        fShader->emitSetupCode(g, "pts", nullptr, wind.c_str(), &vars);

        const char* hullPts = vars.fHullVars.fAlternatePoints;
        if (!hullPts) {
            hullPts = "pts";
        }

        // Visualize the input triangle as upright and equilateral, with a flat base. Paying special
        // attention to wind, we can identify the points as top, bottom-left, and bottom-right.
        //
        // NOTE: We generate the hull in 2 independent invocations, so each invocation designates
        // the corner it will begin with as the top.
        g->codeAppendf("int i = %s > 0 ? sk_InvocationID : 1 - sk_InvocationID;", wind.c_str());
        g->codeAppendf("float2 top = %s[i];", hullPts);
        g->codeAppendf("float2 left = %s[%s > 0 ? (1 - i) * 2 : i + 1];", hullPts, wind.c_str());
        g->codeAppendf("float2 right = %s[%s > 0 ? i + 1 : (1 - i) * 2];", hullPts, wind.c_str());

        // Determine how much to outset the conservative raster hull from each of the three edges.
        g->codeAppend ("float2 leftbloat = float2(top.y > left.y ? +bloat : -bloat, "
                                                 "top.x > left.x ? -bloat : +bloat);");
        g->codeAppend ("float2 rightbloat = float2(right.y > top.y ? +bloat : -bloat, "
                                                  "right.x > top.x ? -bloat : +bloat);");
        g->codeAppend ("float2 downbloat = float2(left.y > right.y ? +bloat : -bloat, "
                                                 "left.x > right.x ? -bloat : +bloat);");

        // Here we generate the conservative raster geometry. It is the convex hull of 3 pixel-size
        // boxes centered on the input points, split between two invocations. This translates to a
        // polygon with either one, two, or three vertices at each input point, depending on how
        // sharp the corner is. For more details on conservative raster, see:
        // https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter42.html
        g->codeAppendf("bool2 left_right_notequal = notEqual(leftbloat, rightbloat);");
        g->codeAppend ("if (all(left_right_notequal)) {");
                           // The top corner will have three conservative raster vertices. Emit the
                           // middle one first to the triangle strip.
        g->codeAppendf(    "%s(top + float2(-leftbloat.y, leftbloat.x));", emitVertexFn);
        g->codeAppend ("}");
        g->codeAppend ("if (any(left_right_notequal)) {");
                           // Second conservative raster vertex for the top corner.
        g->codeAppendf(    "%s(top + rightbloat);", emitVertexFn);
        g->codeAppend ("}");

        // Main interior body of the triangle.
        g->codeAppendf("%s(top + leftbloat);", emitVertexFn);
        g->codeAppendf("%s(right + rightbloat);", emitVertexFn);

        // Here the two invocations diverge. We can't symmetrically divide three triangle points
        // between two invocations, so each does the following:
        //
        // sk_InvocationID=0: Finishes the main interior body of the triangle.
        // sk_InvocationID=1: Remaining two conservative raster vertices for the third corner.
        g->codeAppendf("bool2 right_down_notequal = notEqual(rightbloat, downbloat);");
        g->codeAppend ("if (any(right_down_notequal) || 0 == sk_InvocationID) {");
        g->codeAppendf(    "%s(sk_InvocationID == 0 ? left + leftbloat : right + downbloat);",
                           emitVertexFn);
        g->codeAppend ("}");
        g->codeAppend ("if (all(right_down_notequal) && 0 != sk_InvocationID) {");
        g->codeAppendf(    "%s(right + float2(-rightbloat.y, rightbloat.x));", emitVertexFn);
        g->codeAppend ("}");

        g->configure(InputType::kLines, OutputType::kTriangleStrip, 6, 2);
    }
};

/**
 * Generates a conservative raster hull around a convex quadrilateral. (See comments for RenderPass)
 */
class GSHull4Impl : public GrCCCoverageProcessor::GSImpl {
public:
    GSHull4Impl(std::unique_ptr<Shader> shader) : GSImpl(std::move(shader)) {}

    void onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                             const char* emitVertexFn) const override {
        Shader::GeometryVars vars;
        fShader->emitSetupCode(g, "pts", nullptr, wind.c_str(), &vars);

        const char* hullPts = vars.fHullVars.fAlternatePoints;
        if (!hullPts) {
            hullPts = "pts";
        }

        // Visualize the input (convex) quadrilateral as a square. Paying special attention to wind,
        // we can identify the points by their corresponding corner.
        //
        // NOTE: We split the square down the diagonal from top-right to bottom-left, and generate
        // the hull in two independent invocations. Each invocation designates the corner it will
        // begin with as top-left.
        g->codeAppend ("int i = sk_InvocationID * 2;");
        g->codeAppendf("float2 topleft = %s[i];", hullPts);
        g->codeAppendf("float2 topright = %s[%s > 0 ? i + 1 : 3 - i];", hullPts, wind.c_str());
        g->codeAppendf("float2 bottomleft = %s[%s > 0 ? 3 - i : i + 1];", hullPts, wind.c_str());
        g->codeAppendf("float2 bottomright = %s[2 - i];", hullPts);

        // Determine how much to outset the conservative raster hull from the relevant edges.
        g->codeAppend ("float2 leftbloat = float2(topleft.y > bottomleft.y ? +bloat : -bloat, "
                                                 "topleft.x > bottomleft.x ? -bloat : bloat);");
        g->codeAppend ("float2 upbloat = float2(topright.y > topleft.y ? +bloat : -bloat, "
                                               "topright.x > topleft.x ? -bloat : +bloat);");
        g->codeAppend ("float2 rightbloat = float2(bottomright.y > topright.y ? +bloat : -bloat, "
                                                  "bottomright.x > topright.x ? -bloat : +bloat);");

        // Here we generate the conservative raster geometry. It is the convex hull of 4 pixel-size
        // boxes centered on the input points, split evenly between two invocations. This translates
        // to a polygon with either one, two, or three vertices at each input point, depending on
        // how sharp the corner is. For more details on conservative raster, see:
        // https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter42.html
        g->codeAppendf("bool2 left_up_notequal = notEqual(leftbloat, upbloat);");
        g->codeAppend ("if (all(left_up_notequal)) {");
                           // The top-left corner will have three conservative raster vertices.
                           // Emit the middle one first to the triangle strip.
        g->codeAppendf(    "%s(topleft + float2(-leftbloat.y, leftbloat.x));", emitVertexFn);
        g->codeAppend ("}");
        g->codeAppend ("if (any(left_up_notequal)) {");
                           // Second conservative raster vertex for the top-left corner.
        g->codeAppendf(    "%s(topleft + leftbloat);", emitVertexFn);
        g->codeAppend ("}");

        // Main interior body of this invocation's half of the hull.
        g->codeAppendf("%s(topleft + upbloat);", emitVertexFn);
        g->codeAppendf("%s(bottomleft + leftbloat);", emitVertexFn);
        g->codeAppendf("%s(topright + upbloat);", emitVertexFn);

        // Remaining two conservative raster vertices for the top-right corner.
        g->codeAppendf("bool2 up_right_notequal = notEqual(upbloat, rightbloat);");
        g->codeAppend ("if (any(up_right_notequal)) {");
        g->codeAppendf(    "%s(topright + rightbloat);", emitVertexFn);
        g->codeAppend ("}");
        g->codeAppend ("if (all(up_right_notequal)) {");
        g->codeAppendf(    "%s(topright + float2(-upbloat.y, upbloat.x));", emitVertexFn);
        g->codeAppend ("}");

        g->configure(InputType::kLines, OutputType::kTriangleStrip, 7, 2);
    }
};

/**
 * Generates conservatives around each edge of a triangle. (See comments for RenderPass)
 */
class GSEdgeImpl : public GrCCCoverageProcessor::GSImpl {
public:
    GSEdgeImpl(std::unique_ptr<Shader> shader) : GSImpl(std::move(shader)) {}

    void onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                              const char* emitVertexFn) const override {
        fShader->emitSetupCode(g, "pts", "sk_InvocationID", wind.c_str(), nullptr);

        g->codeAppend ("int nextidx = 2 != sk_InvocationID ? sk_InvocationID + 1 : 0;");
        g->codeAppendf("float2 left = pts[%s > 0 ? sk_InvocationID : nextidx];", wind.c_str());
        g->codeAppendf("float2 right = pts[%s > 0 ? nextidx : sk_InvocationID];", wind.c_str());

        Shader::EmitEdgeDistanceEquation(g, "left", "right", "float3 edge_distance_equation");

        // Which quadrant does the vector from left -> right fall into?
        g->codeAppend ("float2 qlr = sign(right - left);");
        g->codeAppend ("float2x2 outer_pts = float2x2(left - bloat * qlr, right + bloat * qlr);");
        g->codeAppend ("half2 outer_coverage = edge_distance_equation.xy * outer_pts + "
                                              "edge_distance_equation.z;");

        g->codeAppend ("float2 d1 = float2(qlr.y, -qlr.x);");
        g->codeAppend ("float2 d2 = d1;");
        g->codeAppend ("bool aligned = qlr.x == 0 || qlr.y == 0;");
        g->codeAppend ("if (aligned) {");
        g->codeAppend (    "d1 -= qlr;");
        g->codeAppend (    "d2 += qlr;");
        g->codeAppend ("}");

        // Emit the convex hull of 2 pixel-size boxes centered on the endpoints of the edge. Each
        // invocation emits a different edge. Emit negative coverage that subtracts the appropiate
        // amount back out from the hull we drew above.
        g->codeAppend ("if (!aligned) {");
        g->codeAppendf(    "%s(outer_pts[0], outer_coverage[0]);", emitVertexFn);
        g->codeAppend ("}");
        g->codeAppendf("%s(left + bloat * d1, -1);", emitVertexFn);
        g->codeAppendf("%s(left - bloat * d2, 0);", emitVertexFn);
        g->codeAppendf("%s(right + bloat * d2, -1);", emitVertexFn);
        g->codeAppendf("%s(right - bloat * d1, 0);", emitVertexFn);
        g->codeAppend ("if (!aligned) {");
        g->codeAppendf(    "%s(outer_pts[1], outer_coverage[1]);", emitVertexFn);
        g->codeAppend ("}");

        g->configure(InputType::kLines, OutputType::kTriangleStrip, 6, 3);
    }
};

/**
 * Generates conservative rasters around corners. (See comments for RenderPass)
 */
class GSCornerImpl : public GrCCCoverageProcessor::GSImpl {
public:
    GSCornerImpl(std::unique_ptr<Shader> shader, int numCorners)
            : GSImpl(std::move(shader)), fNumCorners(numCorners) {}

    void onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                              const char* emitVertexFn) const override {
        Shader::GeometryVars vars;
        fShader->emitSetupCode(g, "pts", "sk_InvocationID", wind.c_str(), &vars);

        const char* corner = vars.fCornerVars.fPoint;
        SkASSERT(corner);

        g->codeAppendf("%s(%s + float2(-bloat, -bloat));", emitVertexFn, corner);
        g->codeAppendf("%s(%s + float2(-bloat, +bloat));", emitVertexFn, corner);
        g->codeAppendf("%s(%s + float2(+bloat, -bloat));", emitVertexFn, corner);
        g->codeAppendf("%s(%s + float2(+bloat, +bloat));", emitVertexFn, corner);

        g->configure(InputType::kLines, OutputType::kTriangleStrip, 4, fNumCorners);
    }

private:
    const int fNumCorners;
};

void GrCCCoverageProcessor::initGS() {
    SkASSERT(Impl::kGeometryShader == fImpl);
    if (RenderPassIsCubic(fRenderPass)) {
        this->addVertexAttrib("x_or_y_values", kFloat4_GrVertexAttribType); // (See appendMesh.)
        SkASSERT(sizeof(CubicInstance) == this->getVertexStride() * 2);
    } else {
        this->addVertexAttrib("x_or_y_values", kFloat3_GrVertexAttribType); // (See appendMesh.)
        SkASSERT(sizeof(TriangleInstance) == this->getVertexStride() * 2);
    }
    this->setWillUseGeoShader();
}

void GrCCCoverageProcessor::appendGSMesh(GrBuffer* instanceBuffer, int instanceCount,
                                         int baseInstance, SkTArray<GrMesh>* out) const {
    // GSImpl doesn't actually make instanced draw calls. Instead, we feed transposed x,y point
    // values to the GPU in a regular vertex array and draw kLines (see initGS). Then, each vertex
    // invocation receives either the shape's x or y values as inputs, which it forwards to the
    // geometry shader.
    SkASSERT(Impl::kGeometryShader == fImpl);
    GrMesh& mesh = out->emplace_back(GrPrimitiveType::kLines);
    mesh.setNonIndexedNonInstanced(instanceCount * 2);
    mesh.setVertexData(instanceBuffer, baseInstance * 2);
}

GrGLSLPrimitiveProcessor* GrCCCoverageProcessor::createGSImpl(std::unique_ptr<Shader> shadr) const {
    switch (fRenderPass) {
        case RenderPass::kTriangleHulls:
            return new GSHull3Impl(std::move(shadr));
        case RenderPass::kQuadraticHulls:
        case RenderPass::kCubicHulls:
            return new GSHull4Impl(std::move(shadr));
        case RenderPass::kTriangleEdges:
            return new GSEdgeImpl(std::move(shadr));
        case RenderPass::kTriangleCorners:
            return new GSCornerImpl(std::move(shadr), 3);
        case RenderPass::kQuadraticCorners:
        case RenderPass::kCubicCorners:
            return new GSCornerImpl(std::move(shadr), 2);
    }
    SK_ABORT("Invalid RenderPass");
    return nullptr;
}
