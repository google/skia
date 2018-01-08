/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCTriangleShader.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

using Shader = GrCCCoverageProcessor::Shader;

Shader::WindHandling GrCCTriangleShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                                        GrGLSLVarying::Scope scope,
                                                        SkString* code, const char* /*position*/,
                                                        const char* coverage, const char* wind) {
    fCoverageTimesWind.reset(kHalf_GrSLType, scope);
    if (!coverage) {
        varyingHandler->addFlatVarying("wind", &fCoverageTimesWind);
        code->appendf("%s = %s;", OutName(fCoverageTimesWind), wind);
    } else {
        varyingHandler->addVarying("coverage_times_wind", &fCoverageTimesWind);
        code->appendf("%s = %s * %s;", OutName(fCoverageTimesWind), coverage, wind);
    }
    return WindHandling::kHandled;
}

void GrCCTriangleShader::onEmitFragmentCode(GrGLSLPPFragmentBuilder* f,
                                            const char* outputCoverage) const {
    f->codeAppendf("%s = %s;", outputCoverage, fCoverageTimesWind.fsIn());
}

void GrCCTriangleCornerShader::emitSetupCode(GrGLSLVertexGeoBuilder* s, const char* pts,
                                             const char* repetitionID, const char* wind,
                                             GeometryVars* vars) const {
    s->codeAppendf("float2 corner = %s[%s];", pts, repetitionID);
    vars->fCornerVars.fPoint = "corner";

    s->codeAppendf("float2x2 vectors = float2x2(corner - %s[0 != %s ? %s - 1 : 2], "
                                               "corner - %s[2 != %s ? %s + 1 : 0]);",
                                               pts, repetitionID, repetitionID, pts, repetitionID,
                                               repetitionID);

    // Make sure neither vector is 0 to avoid a divide-by-zero. Wind will be zero anyway if this
    // is the case, so whatever we output won't have any effect as long it isn't NaN or Inf.
    s->codeAppend ("for (int i = 0; i < 2; ++i) {");
    s->codeAppend (    "vectors[i] = (vectors[i] != float2(0)) ? vectors[i] : float2(1);");
    s->codeAppend ("}");

    // Find the vector that bisects the region outside the incoming edges. Each edge is
    // responsible to subtract the outside region on its own the side of the bisector.
    s->codeAppendf("float2 leftdir = normalize(vectors[%s > 0 ? 0 : 1]);", wind);
    s->codeAppendf("float2 rightdir = normalize(vectors[%s > 0 ? 1 : 0]);", wind);
    s->codeAppend ("float2 bisect = dot(leftdir, rightdir) >= 0 ? "
                                   "leftdir + rightdir : "
                                   "float2(leftdir.y - rightdir.y, rightdir.x - leftdir.x);");

    // In ccpr we don't calculate exact geometric pixel coverage. What the distance-to-edge
    // method actually finds is coverage inside a logical "AA box", one that is rotated inline
    // with the edge, and in our case, up-scaled to circumscribe the actual pixel. Below we set
    // up transformations into normalized logical AA box space for both incoming edges. These
    // will tell the fragment shader where the corner is located within each edge's AA box.
    s->declareGlobal(fAABoxMatrices);
    s->declareGlobal(fAABoxTranslates);
    s->declareGlobal(fGeoShaderBisects);
    s->codeAppendf("for (int i = 0; i < 2; ++i) {");
    // The X component runs parallel to the edge (i.e. distance to the corner).
    s->codeAppendf(    "float2 n = -vectors[%s > 0 ? i : 1 - i];", wind);
    s->codeAppend (    "float nwidth = (abs(n.x) + abs(n.y)) * (bloat * 2);");
    s->codeAppend (    "n /= nwidth;"); // nwidth != 0 because both vectors != 0.
    s->codeAppendf(    "%s[i][0] = n;", fAABoxMatrices.c_str());
    s->codeAppendf(    "%s[i][0] = -dot(n, corner) + .5;", fAABoxTranslates.c_str());

    // The Y component runs perpendicular to the edge (i.e. distance-to-edge).
    s->codeAppend (    "n = (i == 0) ? float2(-n.y, n.x) : float2(n.y, -n.x);");
    s->codeAppendf(    "%s[i][1] = n;", fAABoxMatrices.c_str());
    s->codeAppendf(    "%s[i][1] = -dot(n, corner) + .5;", fAABoxTranslates.c_str());

    // Translate the bisector into logical AA box space.
    // NOTE: Since the region outside two edges of a convex shape is in [180 deg, 360 deg], the
    // bisector will therefore be in [90 deg, 180 deg]. Or, x >= 0 and y <= 0 in AA box space.
    s->codeAppendf(    "%s[i] = -bisect * %s[i];",
                       fGeoShaderBisects.c_str(), fAABoxMatrices.c_str());
    s->codeAppend ("}");
}

Shader::WindHandling
GrCCTriangleCornerShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                         GrGLSLVarying::Scope scope, SkString* code,
                                         const char* position, const char* coverage,
                                         const char* /*wind*/) {
    SkASSERT(!coverage);

    fCornerLocationInAABoxes.reset(kFloat2x2_GrSLType, scope);
    varyingHandler->addVarying("corner_location_in_aa_boxes", &fCornerLocationInAABoxes);

    fBisectInAABoxes.reset(kFloat2x2_GrSLType, scope);
    varyingHandler->addFlatVarying("bisect_in_aa_boxes", &fBisectInAABoxes);

    code->appendf("for (int i = 0; i < 2; ++i) {");
    code->appendf(    "%s[i] = %s * %s[i] + %s[i];",
                      OutName(fCornerLocationInAABoxes), position, fAABoxMatrices.c_str(),
                      fAABoxTranslates.c_str());
    code->appendf(    "%s[i] = %s[i];", OutName(fBisectInAABoxes), fGeoShaderBisects.c_str());
    code->appendf("}");

    return WindHandling::kNotHandled;
}

void GrCCTriangleCornerShader::onEmitFragmentCode(GrGLSLPPFragmentBuilder* f,
                                                  const char* outputCoverage) const {
    // By the time we reach this shader, the pixel is in the following state:
    //
    //   1. The hull shader has emitted a coverage of 1.
    //   2. Both edges have subtracted the area on their outside.
    //
    // This generally works, but it is a problem for corner pixels. There is a region within
    // corner pixels that is outside both edges at the same time. This means the region has been
    // double subtracted (once by each edge). The purpose of this shader is to fix these corner
    // pixels.
    //
    // More specifically, each edge redoes its coverage analysis so that it only subtracts the
    // outside area that falls on its own side of the bisector line.
    //
    // NOTE: unless the edges fall on multiples of 90 deg from one another, they will have
    // different AA boxes. (For an explanation of AA boxes, see comments in
    // onEmitGeometryShader.) This means the coverage analysis will only be approximate. It
    // seems acceptable, but if we want exact coverage we will need to switch to a more
    // expensive model.
    f->codeAppendf("for (int i = 0; i < 2; ++i) {"); // Loop through both edges.
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
