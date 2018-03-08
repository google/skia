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
        gpArgs->fPositionVar.set(GrVertexAttribTypeToSLType(proc.getAttrib(0).fType),
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
        if (WindMethod::kCrossProduct == proc.fWindMethod) {
            g->codeAppend ("float area_x2 = determinant(float2x2(pts[0] - pts[1], "
                                                                "pts[0] - pts[2]));");
            if (4 == numInputPoints) {
                g->codeAppend ("area_x2 += determinant(float2x2(pts[0] - pts[2], "
                                                               "pts[0] - pts[3]));");
            }
            g->codeAppendf("%s = sign(area_x2);", wind.c_str());
        } else {
            SkASSERT(WindMethod::kInstanceData == proc.fWindMethod);
            SkASSERT(3 == numInputPoints);
            SkASSERT(kFloat4_GrVertexAttribType == proc.getAttrib(0).fType);
            g->codeAppendf("%s = sk_in[0].sk_Position.w;", wind.c_str());
        }

        SkString emitVertexFn;
        SkSTArray<2, GrShaderVar> emitArgs;
        const char* position = emitArgs.emplace_back("position", kFloat2_GrSLType).c_str();
        const char* coverage = emitArgs.emplace_back("coverage", kHalf_GrSLType).c_str();
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
 * Generates conservative rasters around a triangle and its edges, and calculates coverage ramps.
 *
 * Triangle rough outlines are drawn in two steps: (1) draw a conservative raster of the entire
 * triangle, with a coverage of +1, and (2) draw conservative rasters around each edge, with a
 * coverage ramp from -1 to 0. These edge coverage values convert jagged conservative raster edges
 * into smooth, antialiased ones.
 *
 * The final corners get touched up in a later step by GSTriangleCornerImpl.
 */
class GSTriangleImpl : public GrCCCoverageProcessor::GSImpl {
public:
    GSTriangleImpl(std::unique_ptr<Shader> shader) : GSImpl(std::move(shader)) {}

    void onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                              const char* emitVertexFn) const override {
        SkAssertResult(!fShader->emitSetupCode(g, "pts"));

        // Visualize the input triangle as upright and equilateral, with a flat base. Paying special
        // attention to wind, we can identify the points as top, bottom-left, and bottom-right.
        //
        // NOTE: We generate the rasters in 5 independent invocations, so each invocation designates
        // the corner it will begin with as the top.
        g->codeAppendf("int i = (%s > 0 ? sk_InvocationID : 4 - sk_InvocationID) %% 3;",
                       wind.c_str());
        g->codeAppend ("float2 top = pts[i];");
        g->codeAppendf("float2 right = pts[(i + (%s > 0 ? 1 : 2)) %% 3];", wind.c_str());
        g->codeAppendf("float2 left = pts[(i + (%s > 0 ? 2 : 1)) %% 3];", wind.c_str());

        // Determine which direction to outset the conservative raster from each of the three edges.
        g->codeAppend ("float2 leftbloat = sign(top - left);");
        g->codeAppend ("leftbloat = float2(0 != leftbloat.y ? leftbloat.y : leftbloat.x, "
                                          "0 != leftbloat.x ? -leftbloat.x : -leftbloat.y);");

        g->codeAppend ("float2 rightbloat = sign(right - top);");
        g->codeAppend ("rightbloat = float2(0 != rightbloat.y ? rightbloat.y : rightbloat.x, "
                                           "0 != rightbloat.x ? -rightbloat.x : -rightbloat.y);");

        g->codeAppend ("float2 downbloat = sign(left - right);");
        g->codeAppend ("downbloat = float2(0 != downbloat.y ? downbloat.y : downbloat.x, "
                                           "0 != downbloat.x ? -downbloat.x : -downbloat.y);");

        // The triangle's conservative raster has a coverage of +1 all around.
        g->codeAppend ("half4 coverages = half4(+1);");

        // Edges have coverage ramps.
        g->codeAppend ("if (sk_InvocationID >= 2) {"); // Are we an edge?
        Shader::CalcEdgeCoverageAtBloatVertex(g, "top", "right",
                                              "float2(+rightbloat.y, -rightbloat.x)",
                                              "coverages[0]");
        g->codeAppend (    "coverages.yzw = half3(-1, 0, -1 - coverages[0]);");
        // Reassign bloats to characterize a conservative raster around a single edge, rather than
        // the entire triangle.
        g->codeAppend (    "leftbloat = downbloat = -rightbloat;");
        g->codeAppend ("}");

        // These can't be scaled until after we calculate coverage.
        g->codeAppend ("leftbloat *= bloat;");
        g->codeAppend ("rightbloat *= bloat;");
        g->codeAppend ("downbloat *= bloat;");

        // Here we generate the conservative raster geometry. The triangle's conservative raster is
        // the convex hull of 3 pixel-size boxes centered on the input points. This translates to a
        // convex polygon with either one, two, or three vertices at each input point (depending on
        // how sharp the corner is) that we split between two invocations. Edge conservative rasters
        // are convex hulls of 2 pixel-size boxes, one at each endpoint. For more details on
        // conservative raster, see:
        // https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter42.html
        g->codeAppendf("bool2 left_right_notequal = notEqual(leftbloat, rightbloat);");
        g->codeAppend ("if (all(left_right_notequal)) {");
                           // The top corner will have three conservative raster vertices. Emit the
                           // middle one first to the triangle strip.
        g->codeAppendf(    "%s(top + float2(-leftbloat.y, +leftbloat.x), coverages[0]);",
                           emitVertexFn);
        g->codeAppend ("}");
        g->codeAppend ("if (any(left_right_notequal)) {");
                           // Second conservative raster vertex for the top corner.
        g->codeAppendf(    "%s(top + rightbloat, coverages[1]);", emitVertexFn);
        g->codeAppend ("}");

        // Main interior body.
        g->codeAppendf("%s(top + leftbloat, coverages[2]);", emitVertexFn);
        g->codeAppendf("%s(right + rightbloat, coverages[1]);", emitVertexFn);

        // Here the invocations diverge slightly. We can't symmetrically divide three triangle
        // points between two invocations, so each does the following:
        //
        // sk_InvocationID=0: Finishes the main interior body of the triangle hull.
        // sk_InvocationID=1: Remaining two conservative raster vertices for the third hull corner.
        // sk_InvocationID=2..4: Finish the opposite endpoint of their corresponding edge.
        g->codeAppendf("bool2 right_down_notequal = notEqual(rightbloat, downbloat);");
        g->codeAppend ("if (any(right_down_notequal) || 0 == sk_InvocationID) {");
        g->codeAppendf(    "%s(0 == sk_InvocationID ? left + leftbloat : right + downbloat, "
                              "coverages[2]);", emitVertexFn);
        g->codeAppend ("}");
        g->codeAppend ("if (all(right_down_notequal) && 0 != sk_InvocationID) {");
        g->codeAppendf(    "%s(right + float2(-rightbloat.y, +rightbloat.x), coverages[3]);",
                           emitVertexFn);
        g->codeAppend ("}");

        // 5 invocations: 2 triangle hull invocations and 3 edges.
        g->configure(InputType::kLines, OutputType::kTriangleStrip, 6, 5);
    }
};

/**
 * Generates conservative rasters around triangle corners (aka pixel-size boxes) and calculates
 * coverage ramps that fix up the coverage values written by GSTriangleImpl.
 */
class GSTriangleCornerImpl : public GrCCCoverageProcessor::GSImpl {
public:
    GSTriangleCornerImpl(std::unique_ptr<Shader> shader) : GSImpl(std::move(shader)) {}

    void onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                              const char* emitVertexFn) const override {
        SkAssertResult(!fShader->emitSetupCode(g, "pts"));

        g->codeAppendf("float2 corner = pts[sk_InvocationID];");
        g->codeAppendf("float2 left = pts[(sk_InvocationID + (%s > 0 ? 2 : 1)) %% 3];",
                       wind.c_str());
        g->codeAppendf("float2 right = pts[(sk_InvocationID + (%s > 0 ? 1 : 2)) %% 3];",
                       wind.c_str());

        // Find "outbloat" and "crossbloat" at our corner. The outbloat points diagonally out of the
        // triangle, in the direction that should ramp to zero coverage. The crossbloat runs
        // perpindicular to outbloat, and ramps from left-edge coverage to right-edge coverage.
        g->codeAppend ("float2 leftdir = normalize(corner - left);");
        g->codeAppend ("float2 rightdir = normalize(right - corner);");
        g->codeAppend ("float2 outbloat = float2(leftdir.x > rightdir.x ? +1 : -1, "
                                                "leftdir.y > rightdir.y ? +1 : -1);");
        g->codeAppend ("float2 crossbloat = float2(-outbloat.y, +outbloat.x);");

        g->codeAppend ("half2 left_coverages; {");
        Shader::CalcEdgeCoveragesAtBloatVertices(g, "left", "corner", "outbloat", "crossbloat",
                                                 "left_coverages");
        g->codeAppend ("}");

        g->codeAppend ("half2 right_coverages; {");
        Shader::CalcEdgeCoveragesAtBloatVertices(g, "corner", "right", "outbloat", "-crossbloat",
                                                 "right_coverages");
        g->codeAppend ("}");

        // Emit a corner box that erases whatever coverage was written previously, and replaces it
        // using linearly-interpolated values that ramp to zero in bloat vertices that fall outside
        // the triangle.
        //
        // NOTE: Since this is not a linear mapping, it is important that the box's diagonal shared
        // edge points out of the triangle as much as possible.
        g->codeAppendf("%s(corner - crossbloat * bloat, -right_coverages[1]);", emitVertexFn);
        g->codeAppendf("%s(corner + outbloat * bloat, "
                          "-1 - left_coverages[0] - right_coverages[0]);", emitVertexFn);
        g->codeAppendf("%s(corner - outbloat * bloat, 0);", emitVertexFn);
        g->codeAppendf("%s(corner + crossbloat * bloat, -left_coverages[1]);", emitVertexFn);

        g->configure(InputType::kLines, OutputType::kTriangleStrip, 4, 3);
    }
};

/**
 * Generates a conservative raster hull around a convex quadrilateral that encloses a cubic or
 * quadratic, as well as its shared edge.
 */
class GSCurveImpl : public GrCCCoverageProcessor::GSImpl {
public:
    GSCurveImpl(std::unique_ptr<Shader> shader) : GSImpl(std::move(shader)) {}

    void onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                             const char* emitVertexFn) const override {
        const char* hullPts = fShader->emitSetupCode(g, "pts");
        if (!hullPts) {
            hullPts = "pts";
        }

        // Visualize the input (convex) quadrilateral as a square. Paying special attention to wind,
        // we can identify the points by their corresponding corner.
        //
        // NOTE: For the hull we split the square down the diagonal from top-right to bottom-left,
        // and generate it in two independent invocations. All invocations, including the shared
        // edge, designate the corner they will begin with as top-left.
        g->codeAppendf("bool is_shared_edge = (2 == sk_InvocationID);");
        g->codeAppendf("int i = !is_shared_edge ? sk_InvocationID * 2 : (%s > 0 ? 3 : 0);",
                       wind.c_str());
        g->codeAppendf("float2 topleft = %s[i];", hullPts);
        g->codeAppendf("float2 topright = %s[(i + (%s > 0 ? 1 : 3)) & 3];", hullPts, wind.c_str());
        g->codeAppendf("float2 bottomleft = %s[(i + (%s > 0 ? 3 : 1)) & 3];",
                       hullPts, wind.c_str());
        g->codeAppendf("float2 bottomright = %s[(i + 2) & 3];", hullPts);

        // Determine how much to outset the conservative raster hull from the relevant edges.
        g->codeAppend ("float2 leftbloat = sign(topleft - bottomleft) * bloat;");
        g->codeAppend ("leftbloat = float2(0 != leftbloat.y ? leftbloat.y : leftbloat.x, "
                                          "0 != leftbloat.x ? -leftbloat.x : -leftbloat.y);");

        g->codeAppend ("float2 upbloat = sign(topright - topleft) * bloat;");
        g->codeAppend ("upbloat = float2(0 != upbloat.y ? upbloat.y : upbloat.x, "
                                        "0 != upbloat.x ? -upbloat.x : -upbloat.y);");

        g->codeAppend ("float2 rightbloat = sign(bottomright - topright) * bloat;");
        g->codeAppend ("rightbloat = float2(0 != rightbloat.y ? rightbloat.y : rightbloat.x, "
                                           "0 != rightbloat.x ? -rightbloat.x : -rightbloat.y);");

        // The hull raster has a coverage of +1 all around.
        g->codeAppend ("half2 coverages = half2(+1);");

        g->codeAppend ("if (is_shared_edge) {");
                           // On bloat vertices along the shared edge that fall outside the input
                           // points, ramp coverage to 0. We do this by using coverage=-1 to erase
                           // what the hull just wrote.
        g->codeAppend (    "coverages = half2(-1, 0);");
                           // Reassign bloats to characterize a conservative raster around just the
                           // shared edge, rather than the entire hull.
        g->codeAppend (    "leftbloat = rightbloat = -upbloat;");
        g->codeAppend ("}");

        // Here we generate the conservative raster geometry. It is the convex hull of 4 pixel-size
        // boxes centered on the input points, split evenly between two invocations. This translates
        // to a polygon with either one, two, or three vertices at each input point, depending on
        // how sharp the corner is. The shared edge raster is the convex hull of 2 pixel-size boxes,
        // one at each endpoint. For more details on conservative raster, see:
        // https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter42.html
        g->codeAppendf("bool2 left_up_notequal = notEqual(leftbloat, upbloat);");
        g->codeAppend ("if (all(left_up_notequal)) {");
                           // The top-left corner will have three conservative raster vertices.
                           // Emit the middle one first to the triangle strip.
        g->codeAppendf(    "%s(topleft + float2(-leftbloat.y, leftbloat.x), coverages[0]);",
                           emitVertexFn);
        g->codeAppend ("}");
        g->codeAppend ("if (any(left_up_notequal)) {");
                           // Second conservative raster vertex for the top-left corner.
        g->codeAppendf(    "%s(topleft + leftbloat, coverages[1]);", emitVertexFn);
        g->codeAppend ("}");

        g->codeAppendf("%s(topleft + upbloat, coverages[0]);", emitVertexFn);

        g->codeAppend ("if (!is_shared_edge) {");
                           // Main interior body of this invocation's half of the hull.
        g->codeAppendf(    "%s(bottomleft + leftbloat, +1);", emitVertexFn);
        g->codeAppend ("}");

        g->codeAppendf("%s(topright + (is_shared_edge ? rightbloat : upbloat), coverages[1]);",
                       emitVertexFn);

        // Remaining two conservative raster vertices for the top-right corner.
        g->codeAppendf("bool2 up_right_notequal = notEqual(upbloat, rightbloat);");
        g->codeAppend ("if (any(up_right_notequal)) {");
        g->codeAppendf(    "%s(topright + (is_shared_edge ? upbloat : rightbloat), "
                              "coverages[0]);", emitVertexFn);
        g->codeAppend ("}");
        g->codeAppend ("if (all(up_right_notequal)) {");
        g->codeAppendf(    "%s(topright + float2(-upbloat.y, upbloat.x), coverages[0]);",
                           emitVertexFn);
        g->codeAppend ("}");

        // 3 invocations: 2 hull invocations and 1 shared edge.
        g->configure(InputType::kLines, OutputType::kTriangleStrip, 7, 3);
    }
};

void GrCCCoverageProcessor::initGS() {
    SkASSERT(Impl::kGeometryShader == fImpl);
    if (RenderPass::kCubics == fRenderPass || WindMethod::kInstanceData == fWindMethod) {
        SkASSERT(WindMethod::kCrossProduct == fWindMethod || 3 == this->numInputPoints());
        this->addVertexAttrib("x_or_y_values", kFloat4_GrVertexAttribType);
        SkASSERT(sizeof(QuadPointInstance) == this->getVertexStride() * 2);
        SkASSERT(offsetof(QuadPointInstance, fY) == this->getVertexStride());
    } else {
        this->addVertexAttrib("x_or_y_values", kFloat3_GrVertexAttribType);
        SkASSERT(sizeof(TriPointInstance) == this->getVertexStride() * 2);
        SkASSERT(offsetof(TriPointInstance, fY) == this->getVertexStride());
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
        case RenderPass::kTriangles:
            return (GSTriangleSubpass::kHullsAndEdges == fGSTriangleSubpass)
                    ? (GSImpl*) new GSTriangleImpl(std::move(shadr))
                    : (GSImpl*) new GSTriangleCornerImpl(std::move(shadr));
        case RenderPass::kQuadratics:
        case RenderPass::kCubics:
            return new GSCurveImpl(std::move(shadr));
    }
    SK_ABORT("Invalid RenderPass");
    return nullptr;
}
