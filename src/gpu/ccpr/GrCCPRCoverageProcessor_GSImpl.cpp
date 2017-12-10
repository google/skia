/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRCoverageProcessor.h"

#include "GrMesh.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

using Shader = GrCCPRCoverageProcessor::Shader;

/**
 * This class and its subclasses implement the coverage processor with geometry shaders.
 */
class GrCCPRCoverageProcessor::GSImpl : public GrGLSLGeometryProcessor {
protected:
    GSImpl(std::unique_ptr<Shader> shader) : fShader(std::move(shader)) {}

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor&,
                 FPCoordTransformIter&& transformIter) final {
        this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
    }

    void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) final {
        const GrCCPRCoverageProcessor& proc = args.fGP.cast<GrCCPRCoverageProcessor>();

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

    void emitGeometryShader(const GrCCPRCoverageProcessor& proc,
                            GrGLSLVaryingHandler* varyingHandler, GrGLSLGeometryBuilder* g,
                            const char* rtAdjust) const {
        using InputType = GrGLSLGeometryBuilder::InputType;
        using OutputType = GrGLSLGeometryBuilder::OutputType;

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
        const char* coverage = emitArgs.emplace_back("coverage", kHalf_GrSLType).c_str();
        g->emitFunction(kVoid_GrSLType, "emitVertex", emitArgs.count(), emitArgs.begin(), [&]() {
            SkString fnBody;
            fShader->emitVaryings(varyingHandler, &fnBody, position, coverage, wind.c_str());
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

        Shader::GeometryVars vars;
        fShader->emitSetupCode(g, "pts", "sk_InvocationID", wind.c_str(), &vars);
        int maxPoints = this->onEmitGeometryShader(g, wind, emitVertexFn.c_str(), vars);
        g->configure(InputType::kLines, OutputType::kTriangleStrip, maxPoints,
                     fShader->getNumSegments());
    }

    virtual int onEmitGeometryShader(GrGLSLGeometryBuilder*, const GrShaderVar& wind,
                                     const char* emitVertexFn,
                                     const Shader::GeometryVars&) const = 0;

    virtual ~GSImpl() {}

    const std::unique_ptr<Shader> fShader;

    typedef GrGLSLGeometryProcessor INHERITED;
};

class GSHullImpl : public GrCCPRCoverageProcessor::GSImpl {
public:
    GSHullImpl(std::unique_ptr<Shader> shader) : GSImpl(std::move(shader)) {}

    int onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                             const char* emitVertexFn,
                             const Shader::GeometryVars& vars) const override {
        int numSides = fShader->getNumSegments();
        SkASSERT(numSides >= 3);

        const char* hullPts = vars.fHullVars.fAlternatePoints;
        if (!hullPts) {
            hullPts = "pts";
        }

        const char* midpoint = vars.fHullVars.fAlternateMidpoint;
        if (!midpoint) {
            g->codeAppendf("float2 midpoint = %s * float%i(%f);", hullPts, numSides, 1.0/numSides);
            midpoint = "midpoint";
        }

        g->codeAppendf("int previdx = (sk_InvocationID + %i) %% %i, "
                           "nextidx = (sk_InvocationID + 1) %% %i;",
                       numSides - 1, numSides, numSides);

        g->codeAppendf("float2 self = %s[sk_InvocationID];"
                       "int leftidx = %s > 0 ? previdx : nextidx;"
                       "int rightidx = %s > 0 ? nextidx : previdx;",
                       hullPts, wind.c_str(), wind.c_str());

        // Which quadrant does the vector from self -> right fall into?
        g->codeAppendf("float2 right = %s[rightidx];", hullPts);
        if (3 == numSides) {
            // TODO: evaluate perf gains.
            g->codeAppend ("float2 qsr = sign(right - self);");
        } else {
            SkASSERT(4 == numSides);
            g->codeAppendf("float2 diag = %s[(sk_InvocationID + 2) %% 4];", hullPts);
            g->codeAppend ("float2 qsr = sign((right != self ? right : diag) - self);");
        }

        // Which quadrant does the vector from left -> self fall into?
        g->codeAppendf("float2 qls = sign(self - %s[leftidx]);", hullPts);

        // d2 just helps us reduce triangle counts with orthogonal, axis-aligned lines.
        // TODO: evaluate perf gains.
        const char* dr2 = "dr";
        if (3 == numSides) {
            // TODO: evaluate perf gains.
            g->codeAppend ("float2 dr = float2(qsr.y != 0 ? +qsr.y : +qsr.x, "
                                              "qsr.x != 0 ? -qsr.x : +qsr.y);");
            g->codeAppend ("float2 dr2 = float2(qsr.y != 0 ? +qsr.y : -qsr.x, "
                                               "qsr.x != 0 ? -qsr.x : -qsr.y);");
            g->codeAppend ("float2 dl = float2(qls.y != 0 ? +qls.y : +qls.x, "
                                              "qls.x != 0 ? -qls.x : +qls.y);");
            dr2 = "dr2";
        } else {
            g->codeAppend ("float2 dr = float2(qsr.y != 0 ? +qsr.y : 1, "
                                              "qsr.x != 0 ? -qsr.x : 1);");
            g->codeAppend ("float2 dl = (qls == float2(0)) ? dr : "
                                       "float2(qls.y != 0 ? +qls.y : 1, qls.x != 0 ? -qls.x : 1);");
        }
        g->codeAppendf("bool2 dnotequal = notEqual(%s, dl);", dr2);

        // Emit one third of what is the convex hull of pixel-size boxes centered on the vertices.
        // Each invocation emits a different third.
        g->codeAppendf("%s(right + bloat * dr, 1);", emitVertexFn);
        g->codeAppendf("%s(%s, 1);", emitVertexFn, midpoint);
        g->codeAppendf("%s(self + bloat * %s, 1);", emitVertexFn, dr2);
        g->codeAppend ("if (any(dnotequal)) {");
        g->codeAppendf(    "%s(self + bloat * dl, 1);", emitVertexFn);
        g->codeAppend ("}");
        g->codeAppend ("if (all(dnotequal)) {");
        g->codeAppendf(    "%s(self + bloat * float2(-dl.y, dl.x), 1);", emitVertexFn);
        g->codeAppend ("}");
        g->endPrimitive();

        return 5;
    }
};

class GSEdgeImpl : public GrCCPRCoverageProcessor::GSImpl {
public:
    GSEdgeImpl(std::unique_ptr<Shader> shader) : GSImpl(std::move(shader)) {}

    int onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                             const char* emitVertexFn,
                             const Shader::GeometryVars&) const override {
        int numSides = fShader->getNumSegments();

        g->codeAppendf("int nextidx = (sk_InvocationID + 1) %% %i;", numSides);
        g->codeAppendf("float2 left = pts[%s > 0 ? sk_InvocationID : nextidx], "
                                     "right = pts[%s > 0 ? nextidx : sk_InvocationID];",
                                     wind.c_str(), wind.c_str());

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
        g->endPrimitive();

        return 6;
    }
};

class GSCornerImpl : public GrCCPRCoverageProcessor::GSImpl {
public:
    GSCornerImpl(std::unique_ptr<Shader> shader) : GSImpl(std::move(shader)) {}

    int onEmitGeometryShader(GrGLSLGeometryBuilder* g, const GrShaderVar& wind,
                             const char* emitVertexFn,
                             const Shader::GeometryVars& vars) const override {
        const char* corner = vars.fCornerVars.fPoint;
        SkASSERT(corner);

        g->codeAppendf("%s(%s + float2(-bloat, -bloat), 1);", emitVertexFn, corner);
        g->codeAppendf("%s(%s + float2(-bloat, +bloat), 1);", emitVertexFn, corner);
        g->codeAppendf("%s(%s + float2(+bloat, -bloat), 1);", emitVertexFn, corner);
        g->codeAppendf("%s(%s + float2(+bloat, +bloat), 1);", emitVertexFn, corner);
        g->endPrimitive();

        return 4;
    }
};

void GrCCPRCoverageProcessor::initGS() {
    if (RenderPassIsCubic(fRenderPass)) {
        this->addVertexAttrib("x_or_y_values", kFloat4_GrVertexAttribType); // (See appendMesh.)
        SkASSERT(sizeof(CubicInstance) == this->getVertexStride() * 2);
    } else {
        this->addVertexAttrib("x_or_y_values", kFloat3_GrVertexAttribType); // (See appendMesh.)
        SkASSERT(sizeof(TriangleInstance) == this->getVertexStride() * 2);
    }
    this->setWillUseGeoShader();
}

GrGLSLPrimitiveProcessor* GrCCPRCoverageProcessor::CreateGSImpl(std::unique_ptr<Shader> shader) {
    switch (shader->getGeometryType()) {
        case Shader::GeometryType::kHull:
            return new GSHullImpl(std::move(shader));
        case Shader::GeometryType::kEdges:
            return new GSEdgeImpl(std::move(shader));
        case Shader::GeometryType::kCorners:
            return new GSCornerImpl(std::move(shader));
    }
    SK_ABORT("Unexpected Shader::GeometryType.");
    return nullptr;
}

void GrCCPRCoverageProcessor::appendGSMesh(GrBuffer* instanceBuffer, int instanceCount,
                                           int baseInstance, SkTArray<GrMesh, true>* out) {
    // GSImpl doesn't actually make instanced draw calls. Instead, we feed transposed x,y point
    // values to the GPU in a regular vertex array and draw kLines (see initGS). Then, each vertex
    // invocation receives either the shape's x or y values as inputs, which it forwards to the
    // geometry shader.
    GrMesh& mesh = out->emplace_back(GrPrimitiveType::kLines);
    mesh.setNonIndexedNonInstanced(instanceCount * 2);
    mesh.setVertexData(instanceBuffer, baseInstance * 2);
}
