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

void GrCCPRTriangleProcessor::onEmitVertexShader(const GrCCPRCoverageProcessor& proc,
                                                 GrGLSLVertexBuilder* v,
                                                 const TexelBufferHandle& pointsBuffer,
                                                 const char* atlasOffset, const char* rtAdjust,
                                                 GrGPArgs* gpArgs) const {
    v->codeAppend ("float2 self = ");
    v->appendTexelFetch(pointsBuffer,
                        SkStringPrintf("%s[sk_VertexID]", proc.instanceAttrib()).c_str());
    v->codeAppendf(".xy + %s;", atlasOffset);
    gpArgs->fPositionVar.set(kFloat2_GrSLType, "self");
}

void GrCCPRTriangleProcessor::defineInputVertices(GrGLSLGeometryBuilder* g) const {
    // Prepend in_vertices at the start of the shader.
    g->codePrependf("float3x2 in_vertices = float3x2(sk_in[0].sk_Position.xy, "
                                                    "sk_in[1].sk_Position.xy, "
                                                    "sk_in[2].sk_Position.xy);");
}

void GrCCPRTriangleProcessor::emitWind(GrGLSLGeometryBuilder* g, const char* /*rtAdjust*/,
                                       const char* outputWind) const {
    // We will define in_vertices in defineInputVertices.
    g->codeAppendf("%s = sign(determinant(float2x2(in_vertices[1] - in_vertices[0], "
                                                  "in_vertices[2] - in_vertices[0])));",
                   outputWind);
}

void GrCCPRTriangleHullAndEdgeProcessor::onEmitGeometryShader(GrGLSLGeometryBuilder* g,
                                                              const char* emitVertexFn,
                                                              const char* wind,
                                                              const char* rtAdjust) const {
    this->defineInputVertices(g);
    int maxOutputVertices = 0;

    if (GeometryType::kEdges != fGeometryType) {
        maxOutputVertices += this->emitHullGeometry(g, emitVertexFn, "in_vertices", 3,
                                                    "sk_InvocationID");
    }

    if (GeometryType::kHulls != fGeometryType) {
        g->codeAppend ("int edgeidx0 = sk_InvocationID, "
                           "edgeidx1 = (edgeidx0 + 1) % 3;");
        g->codeAppendf("float2 edgept0 = in_vertices[%s > 0 ? edgeidx0 : edgeidx1];", wind);
        g->codeAppendf("float2 edgept1 = in_vertices[%s > 0 ? edgeidx1 : edgeidx0];", wind);

        maxOutputVertices += this->emitEdgeGeometry(g, emitVertexFn, "edgept0", "edgept1");
    }

    g->configure(GrGLSLGeometryBuilder::InputType::kTriangles,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip,
                 maxOutputVertices, 3);
}

void GrCCPRTriangleCornerProcessor::onEmitGeometryShader(GrGLSLGeometryBuilder* g,
                                                         const char* emitVertexFn, const char* wind,
                                                         const char* rtAdjust) const {
    this->defineInputVertices(g);

    g->codeAppend ("float2 corner = in_vertices[sk_InvocationID];");
    g->codeAppend ("float2x2 vectors = float2x2(corner - in_vertices[(sk_InvocationID + 2) % 3], "
                                               "corner - in_vertices[(sk_InvocationID + 1) % 3]);");

    // Make sure neither vector is 0 in order to avoid a divide-by-zero. Wind will be zero anyway if
    // this is the case, so whatever we output won't have any effect as long it isn't NaN or Inf.
    g->codeAppendf("for (int i = 0; i < 2; ++i) {");
    g->codeAppendf(    "vectors[i] = any(notEqual(vectors[i], float2(0))) ? "
                                    "vectors[i] : float2(1);");
    g->codeAppendf("}");

    // Find the vector that bisects the region outside the incoming edges. Each edge is responsible
    // to subtract the outside region on its own the side of the bisector.
    g->codeAppendf("float2 leftdir = normalize(vectors[%s > 0 ? 0 : 1]);", wind);
    g->codeAppendf("float2 rightdir = normalize(vectors[%s > 0 ? 1 : 0]);", wind);
    g->codeAppendf("float2 bisect = dot(leftdir, rightdir) >= 0 ? leftdir + rightdir : "
                                        "float2(leftdir.y - rightdir.y, rightdir.x - leftdir.x);");

    // In ccpr we don't calculate exact geometric pixel coverage. What the distance-to-edge method
    // actually finds is coverage inside a logical "AA box", one that is rotated inline with the
    // edge, and in our case, up-scaled to circumscribe the actual pixel. Below we set up
    // transformations into normalized logical AA box space for both incoming edges. These will tell
    // the fragment shader where the corner is located within each edge's AA box.
    g->declareGlobal(fAABoxMatrices);
    g->declareGlobal(fAABoxTranslates);
    g->declareGlobal(fGeoShaderBisects);
    g->codeAppendf("for (int i = 0; i < 2; ++i) {");
    // The X component runs parallel to the edge (i.e. distance to the corner).
    g->codeAppendf(    "float2 n = -vectors[%s > 0 ? i : 1 - i];", wind);
    g->codeAppendf(    "float nwidth = dot(abs(n), bloat) * 2;");
    g->codeAppendf(    "n /= nwidth;"); // nwidth != 0 because both vectors != 0.
    g->codeAppendf(    "%s[i][0] = n;", fAABoxMatrices.c_str());
    g->codeAppendf(    "%s[i][0] = -dot(n, corner) + .5;", fAABoxTranslates.c_str());

    // The Y component runs perpendicular to the edge (i.e. distance-to-edge).
    // NOTE: once we are back in device space and bloat.x == bloat.y, we will not need to find and
    // divide by nwidth a second time.
    g->codeAppendf(    "n = (i == 0) ? float2(-n.y, n.x) : float2(n.y, -n.x);");
    g->codeAppendf(    "nwidth = dot(abs(n), bloat) * 2;");
    g->codeAppendf(    "n /= nwidth;");
    g->codeAppendf(    "%s[i][1] = n;", fAABoxMatrices.c_str());
    g->codeAppendf(    "%s[i][1] = -dot(n, corner) + .5;", fAABoxTranslates.c_str());

    // Translate the bisector into logical AA box space.
    // NOTE: Since the region outside two edges of a convex shape is in [180 deg, 360 deg], the
    // bisector will therefore be in [90 deg, 180 deg]. Or, x >= 0 and y <= 0 in AA box space.
    g->codeAppendf(    "%s[i] = -bisect * %s[i];",
                       fGeoShaderBisects.c_str(), fAABoxMatrices.c_str());
    g->codeAppendf("}");

    int numVertices = this->emitCornerGeometry(g, emitVertexFn, "corner");

    g->configure(GrGLSLGeometryBuilder::InputType::kTriangles,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip,
                 numVertices, 3);
}

void GrCCPRTriangleCornerProcessor::emitPerVertexGeometryCode(SkString* fnBody,
                                                              const char* position,
                                                              const char* /*coverage*/,
                                                              const char* wind) const {
    fnBody->appendf("for (int i = 0; i < 2; ++i) {");
    fnBody->appendf(    "%s[i] = %s * %s[i] + %s[i];",
                        fCornerLocationInAABoxes.gsOut(), position, fAABoxMatrices.c_str(),
                        fAABoxTranslates.c_str());
    fnBody->appendf(    "%s[i] = %s[i];", fBisectInAABoxes.gsOut(), fGeoShaderBisects.c_str());
    fnBody->appendf("}");
}

void GrCCPRTriangleCornerProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                       const char* outputCoverage) const {
    // By the time we reach this shader, the pixel is in the following state:
    //
    //   1. The hull shader has emitted a coverage of 1.
    //   2. Both edges have subtracted the area on their outside.
    //
    // This generally works, but it is a problem for corner pixels. There is a region within corner
    // pixels that is outside both edges at the same time. This means the region has been double
    // subtracted (once by each edge). The purpose of this shader is to fix these corner pixels.
    //
    // More specifically, each edge redoes its coverage analysis so that it only subtracts the
    // outside area that falls on its own side of the bisector line.
    //
    // NOTE: unless the edges fall on multiples of 90 deg from one another, they will have different
    // AA boxes. (For an explanation of AA boxes, see comments in onEmitGeometryShader.) This means
    // the coverage analysis will only be approximate. It seems acceptable, but if we want exact
    // coverage we will need to switch to a more expensive model.
    f->codeAppendf("%s = 0;", outputCoverage);

    // Loop through both edges.
    f->codeAppendf("for (int i = 0; i < 2; ++i) {");
    f->codeAppendf(    "half2 corner = %s[i];", fCornerLocationInAABoxes.fsIn());
    f->codeAppendf(    "half2 bisect = %s[i];", fBisectInAABoxes.fsIn());

    // Find the point at which the bisector exits the logical AA box.
    // (The inequality works because bisect.x is known >= 0 and bisect.y is known <= 0.)
    f->codeAppendf(    "half2 d = half2(1 - corner.x, -corner.y);");
    f->codeAppendf(    "half T = d.y * bisect.x >= d.x * bisect.y ? d.y / bisect.y "
                                                                 ": d.x / bisect.x;");
    f->codeAppendf(    "half2 exit = corner + bisect * T;");

    // These lines combined (and the final multiply by .5) accomplish the following:
    //   1. Add back the area beyond the corner that was subtracted out previously.
    //   2. Subtract out the area beyond the corner, but under the bisector.
    // The other edge will take care of the area on its own side of the bisector.
    f->codeAppendf(    "%s += (2 - corner.x - exit.x) * corner.y;", outputCoverage);
    f->codeAppendf(    "%s += (corner.x - 1) * exit.y;", outputCoverage);
    f->codeAppendf("}");

    f->codeAppendf("%s *= .5;", outputCoverage);
}
