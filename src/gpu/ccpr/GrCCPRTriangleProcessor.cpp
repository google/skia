/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRTriangleProcessor.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryShaderBuilder.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

void GrCCPRTriangleProcessor::onEmitVertexShader(const GrCCPRCoverageProcessor& cp,
                                                 GrGLSLVertexBuilder* v, const char* rtAdjustName,
                                                 const TexelBufferHandle& pointsBuffer,
                                                 GrGPArgs* gpArgs) const {
    v->codeAppendf("ivec3 vertex_indices = %s.wyz;", cp.instanceAttrib());
    v->codeAppend ("highp vec2 self = ");
    v->appendTexelFetch(pointsBuffer, "vertex_indices[sk_VertexID]");
    v->codeAppend (".xy + atlasoffset;");
    gpArgs->fPositionVar.set(kVec2f_GrSLType, "self");
}

void GrCCPRTriangleProcessor::onEmitGeometryShader(GrGLSLGeometryBuilder* g,
                                                   const char* emitVertexFn, const char* wind,
                                                   const char* rtAdjustName) const {
    int maxVertices = 0;

    if (GeometryType::kEdges != fGeometryType) {
        maxVertices += this->emitHullGeometry(g, 3, "sk_InvocationID", emitVertexFn, "P");
    }

    if (GeometryType::kHulls != fGeometryType) {
        g->codeAppend ("int edgeidx0 = sk_InvocationID, "
                           "edgeidx1 = (edgeidx0 + 1) % 3;");
        g->codeAppendf("highp vec2 edgept0 = P[%s > 0 ? edgeidx0 : edgeidx1];", wind);
        g->codeAppendf("highp vec2 edgept1 = P[%s > 0 ? edgeidx1 : edgeidx0];", wind);

        maxVertices += this->emitEdgeGeometry(g, emitVertexFn, "edgept0", "edgept1");
    }

    g->configure(GrGLSLGeometryBuilder::InputType::kTriangles,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip,
                 maxVertices, 3);
}

void GrCCPRTriangleCornerProcessor::onEmitVertexShader(const GrCCPRCoverageProcessor& cp,
                                                       GrGLSLVertexBuilder* v,
                                                       const char* rtAdjustName,
                                                       const TexelBufferHandle& pointsBuffer,
                                                       GrGPArgs* gpArgs) const {
    this->INHERITED::onEmitVertexShader(cp, v, rtAdjustName, pointsBuffer, gpArgs);

    // Fetch and transform the next point in the triangle.
    v->codeAppend ("highp vec2 next = ");
    v->appendTexelFetch(pointsBuffer, "vertex_indices[(sk_VertexID + 1) % 3]");
    v->codeAppend (".xy + atlasoffset;");

    // Find the plane that gives distance from the [self -> next] edge, normalized to its AA
    // bloat width.
    v->codeAppend ("highp vec2 n = vec2(next.y - self.y, self.x - next.x);");
    v->codeAppendf("highp vec2 d = n * mat2(self + %f * sign(n), "
                                           "self - %f * sign(n));", kAABloatRadius, kAABloatRadius);

    // Clamp for when n=0. (wind=0 when n=0, so as long as we don't get Inf or NaN we are fine.)
    v->codeAppendf("%s.xy = n / max(d[0] - d[1], 1e-30);", fEdgeDistance.vsOut());
    v->codeAppendf("%s.z = -dot(%s.xy, self);", fEdgeDistance.vsOut(), fEdgeDistance.vsOut());

    // Emit device coords to geo shader.
    v->codeAppendf("%s = self;", fDevCoord.vsOut());
}

void GrCCPRTriangleCornerProcessor::onEmitGeometryShader(GrGLSLGeometryBuilder* g,
                                                         const char* emitVertexFn, const char* wind,
                                                         const char* rtAdjustName) const {
    g->codeAppend ("highp vec2 self = sk_in[sk_InvocationID].gl_Position.xy;");
    g->codeAppendf("%s(self + vec2(-bloat.x, -bloat.y), 1);", emitVertexFn);
    g->codeAppendf("%s(self + vec2(-bloat.x, +bloat.y), 1);", emitVertexFn);
    g->codeAppendf("%s(self + vec2(+bloat.x, -bloat.y), 1);", emitVertexFn);
    g->codeAppendf("%s(self + vec2(+bloat.x, +bloat.y), 1);", emitVertexFn);
    g->codeAppend ("EndPrimitive();");

    g->configure(GrGLSLGeometryBuilder::InputType::kTriangles,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip,
                 4, 3);
}

void GrCCPRTriangleCornerProcessor::emitPerVertexGeometryCode(SkString* fnBody,
                                                              const char* position,
                                                              const char* coverage,
                                                              const char* wind) const {
    fnBody->appendf("%s.xy = %s[(sk_InvocationID + 1) %% 3];",
                    fNeighbors.gsOut(), fDevCoord.gsIn());
    fnBody->appendf("%s.zw = %s[(sk_InvocationID + 2) %% 3];",
                    fNeighbors.gsOut(), fDevCoord.gsIn());
    fnBody->appendf("%s = mat3(%s[(sk_InvocationID + 2) %% 3], "
                              "%s[sk_InvocationID], "
                              "%s[(sk_InvocationID + 1) %% 3]) * %s;",
                    fEdgeDistances.gsOut(), fEdgeDistance.gsIn(), fEdgeDistance.gsIn(),
                    fEdgeDistance.gsIn(), wind);

    // Otherwise, fEdgeDistances = mat3(...) * sign(wind * rtAdjust.x * rdAdjust.z).
    GR_STATIC_ASSERT(kTopLeft_GrSurfaceOrigin == GrCCPRCoverageProcessor::kAtlasOrigin);

    fnBody->appendf("%s = sk_InvocationID;", fCornerIdx.gsOut());
}

void GrCCPRTriangleCornerProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                       const char* outputCoverage) const {
    // FIXME: Adreno breaks if we don't put the frag coord in an intermediate highp variable.
    f->codeAppendf("highp vec2 fragcoord = sk_FragCoord.xy;");

    // Approximate coverage by tracking where 4 horizontal lines enter and leave the triangle.
    GrShaderVar samples("samples", kVec4f_GrSLType, GrShaderVar::kNonArray,
                        kHigh_GrSLPrecision);
    f->declareGlobal(samples);
    f->codeAppendf("%s = fragcoord.y + vec4(-0.375, -0.125, 0.125, 0.375);", samples.c_str());

    GrShaderVar leftedge("leftedge", kVec4f_GrSLType, GrShaderVar::kNonArray,
                         kHigh_GrSLPrecision);
    f->declareGlobal(leftedge);
    f->codeAppendf("%s = vec4(fragcoord.x - 0.5);", leftedge.c_str());

    GrShaderVar rightedge("rightedge", kVec4f_GrSLType, GrShaderVar::kNonArray,
                          kHigh_GrSLPrecision);
    f->declareGlobal(rightedge);
    f->codeAppendf("%s = vec4(fragcoord.x + 0.5);", rightedge.c_str());

    SkString sampleEdgeFn;
    GrShaderVar edgeArg("edge_distance", kVec3f_GrSLType, GrShaderVar::kNonArray,
                        kHigh_GrSLPrecision);
    f->emitFunction(kVoid_GrSLType, "sampleEdge", 1, &edgeArg, [&]() {
        SkString b;
        b.appendf("highp float m = abs(%s.x) < 1e-3 ? 1e18 : -1 / %s.x;",
                  edgeArg.c_str(), edgeArg.c_str());
        b.appendf("highp vec4 edge = m * (%s.y * samples + %s.z);",
                  edgeArg.c_str(), edgeArg.c_str());
        b.appendf("if (%s.x <= 1e-3 || (abs(%s.x) < 1e-3 && %s.y > 0)) {",
                  edgeArg.c_str(), edgeArg.c_str(), edgeArg.c_str());
        b.appendf(    "%s = max(%s, edge);", leftedge.c_str(), leftedge.c_str());
        b.append ("} else {");
        b.appendf(    "%s = min(%s, edge);", rightedge.c_str(), rightedge.c_str());
        b.append ("}");
        return b;
    }().c_str(), &sampleEdgeFn);

    // See if the previous neighbor already handled this pixel.
    f->codeAppendf("if (all(lessThan(abs(fragcoord - %s.zw), vec2(%f)))) {",
                   fNeighbors.fsIn(), kAABloatRadius);
    // Handle the case where all 3 corners defer to the previous neighbor.
    f->codeAppendf(    "if (%s != 0 || !all(lessThan(abs(fragcoord - %s.xy), vec2(%f)))) {",
                       fCornerIdx.fsIn(), fNeighbors.fsIn(), kAABloatRadius);
    f->codeAppend (        "discard;");
    f->codeAppend (    "}");
    f->codeAppend ("}");

    // Erase what the hull and two egdes wrote at this corner in previous shaders (the two .5's
    // for the edges and the -1 for the hull cancel each other out).
    f->codeAppendf("%s = dot(vec3(fragcoord, 1) * mat2x3(%s), vec2(1));",
                   outputCoverage, fEdgeDistances.fsIn());

    // Sample the two egdes at this corner.
    f->codeAppendf("%s(%s[0]);", sampleEdgeFn.c_str(), fEdgeDistances.fsIn());
    f->codeAppendf("%s(%s[1]);", sampleEdgeFn.c_str(), fEdgeDistances.fsIn());

    // Handle the opposite edge if the next neighbor will defer to us.
    f->codeAppendf("if (all(lessThan(abs(fragcoord - %s.xy), vec2(%f)))) {",
                   fNeighbors.fsIn(), kAABloatRadius);
    // Erase the coverage the opposite edge wrote to this corner.
    f->codeAppendf(    "%s += dot(%s[2], vec3(fragcoord, 1)) + 0.5;",
                       outputCoverage, fEdgeDistances.fsIn());
    // Sample the opposite edge.
    f->codeAppendf(    "%s(%s[2]);", sampleEdgeFn.c_str(), fEdgeDistances.fsIn());
    f->codeAppend ("}");

    f->codeAppendf("highp vec4 widths = max(%s - %s, 0);", rightedge.c_str(), leftedge.c_str());
    f->codeAppendf("%s += dot(widths, vec4(0.25));", outputCoverage);
}
