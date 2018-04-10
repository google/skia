/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCCoverageProcessor.h"

#include "GrGpuCommandBuffer.h"
#include "GrOpFlushState.h"
#include "SkMakeUnique.h"
#include "ccpr/GrCCConicShader.h"
#include "ccpr/GrCCCubicShader.h"
#include "ccpr/GrCCQuadraticShader.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

class GrCCCoverageProcessor::TriangleShader : public GrCCCoverageProcessor::Shader {
    void onEmitVaryings(GrGLSLVaryingHandler* varyingHandler, GrGLSLVarying::Scope scope,
                        SkString* code, const char* position, const char* coverage,
                        const char* cornerCoverage) override {
        if (!cornerCoverage) {
            fCoverages.reset(kHalf_GrSLType, scope);
            varyingHandler->addVarying("coverage", &fCoverages);
            code->appendf("%s = %s;", OutName(fCoverages), coverage);
        } else {
            fCoverages.reset(kHalf3_GrSLType, scope);
            varyingHandler->addVarying("coverages", &fCoverages);
            code->appendf("%s = half3(%s, %s);", OutName(fCoverages), coverage, cornerCoverage);
        }
    }

    void onEmitFragmentCode(GrGLSLFPFragmentBuilder* f, const char* outputCoverage) const override {
        if (kHalf_GrSLType == fCoverages.type()) {
            f->codeAppendf("%s = %s;", outputCoverage, fCoverages.fsIn());
        } else {
            f->codeAppendf("%s = %s.z * %s.y + %s.x;",
                           outputCoverage, fCoverages.fsIn(), fCoverages.fsIn(), fCoverages.fsIn());
        }
    }

    GrGLSLVarying fCoverages;
};

void GrCCCoverageProcessor::Shader::EmitEdgeDistanceEquation(GrGLSLVertexGeoBuilder* s,
                                                             const char* leftPt,
                                                             const char* rightPt,
                                                             const char* outputDistanceEquation) {
    s->codeAppendf("float2 n = float2(%s.y - %s.y, %s.x - %s.x);",
                   rightPt, leftPt, leftPt, rightPt);
    s->codeAppend ("float nwidth = (abs(n.x) + abs(n.y)) * (bloat * 2);");
    // When nwidth=0, wind must also be 0 (and coverage * wind = 0). So it doesn't matter what we
    // come up with here as long as it isn't NaN or Inf.
    s->codeAppend ("n /= (0 != nwidth) ? nwidth : 1;");
    s->codeAppendf("%s = float3(-n, dot(n, %s) - .5);", outputDistanceEquation, leftPt);
}

void GrCCCoverageProcessor::Shader::CalcEdgeCoverageAtBloatVertex(GrGLSLVertexGeoBuilder* s,
                                                                  const char* leftPt,
                                                                  const char* rightPt,
                                                                  const char* rasterVertexDir,
                                                                  const char* outputCoverage) {
    // Here we find an edge's coverage at one corner of a conservative raster bloat box whose center
    // falls on the edge in question. (A bloat box is axis-aligned and the size of one pixel.) We
    // always set up coverage so it is -1 at the outermost corner, 0 at the innermost, and -.5 at
    // the center. Interpolated, these coverage values convert jagged conservative raster edges into
    // smooth antialiased edges.
    //
    // d1 == (P + sign(n) * bloat) dot n                   (Distance at the bloat box vertex whose
    //    == P dot n + (abs(n.x) + abs(n.y)) * bloatSize    coverage=-1, where the bloat box is
    //                                                      centered on P.)
    //
    // d0 == (P - sign(n) * bloat) dot n                   (Distance at the bloat box vertex whose
    //    == P dot n - (abs(n.x) + abs(n.y)) * bloatSize    coverage=0, where the bloat box is
    //                                                      centered on P.)
    //
    // d == (P + rasterVertexDir * bloatSize) dot n        (Distance at the bloat box vertex whose
    //   == P dot n + (rasterVertexDir dot n) * bloatSize   coverage we wish to calculate.)
    //
    // coverage == -(d - d0) / (d1 - d0)                   (coverage=-1 at d=d1; coverage=0 at d=d0)
    //
    //          == (rasterVertexDir dot n) / (abs(n.x) + abs(n.y)) * -.5 - .5
    //
    s->codeAppendf("float2 n = float2(%s.y - %s.y, %s.x - %s.x);",
                   rightPt, leftPt, leftPt, rightPt);
    s->codeAppend ("float nwidth = abs(n.x) + abs(n.y);");
    s->codeAppendf("float t = dot(%s, n);", rasterVertexDir);
    // The below conditional guarantees we get exactly 1 on the divide when nwidth=t (in case the
    // GPU divides by multiplying by the reciprocal?) It also guards against NaN when nwidth=0.
    s->codeAppendf("%s = (abs(t) != nwidth ? t / nwidth : sign(t)) * -.5 - .5;", outputCoverage);
}

void GrCCCoverageProcessor::Shader::CalcEdgeCoveragesAtBloatVertices(GrGLSLVertexGeoBuilder* s,
                                                                     const char* leftPt,
                                                                     const char* rightPt,
                                                                     const char* bloatDir1,
                                                                     const char* bloatDir2,
                                                                     const char* outputCoverages) {
    // See comments in CalcEdgeCoverageAtBloatVertex.
    s->codeAppendf("float2 n = float2(%s.y - %s.y, %s.x - %s.x);",
                   rightPt, leftPt, leftPt, rightPt);
    s->codeAppend ("float nwidth = abs(n.x) + abs(n.y);");
    s->codeAppendf("float2 t = n * float2x2(%s, %s);", bloatDir1, bloatDir2);
    s->codeAppendf("for (int i = 0; i < 2; ++i) {");
    s->codeAppendf(    "%s[i] = (abs(t[i]) != nwidth ? t[i] / nwidth : sign(t[i])) * -.5 - .5;",
                       outputCoverages);
    s->codeAppendf("}");
}

void GrCCCoverageProcessor::Shader::CalcCornerAttenuation(GrGLSLVertexGeoBuilder* s,
                                                          const char* leftDir, const char* rightDir,
                                                          const char* outputAttenuation) {
    // obtuseness = cos(corner_angle)  if corner_angle > 90 degrees
    //                              0  if corner_angle <= 90 degrees
    s->codeAppendf("half obtuseness = max(dot(%s, %s), 0);", leftDir, rightDir);

    // axis_alignedness = 1  when the leftDir/rightDir bisector is aligned with the x- or y-axis
    //                    0  when the bisector falls on a 45 degree angle
    //                    (i.e. 1 - tan(angle_to_nearest_axis))
    s->codeAppendf("half2 abs_bisect = abs(%s - %s);", leftDir, rightDir);
    s->codeAppend ("half axis_alignedness = 1 - min(abs_bisect.y, abs_bisect.x) / "
                                               "max(abs_bisect.x, abs_bisect.y);");

    // ninety_degreesness = sin^2(corner_angle)
    // sin^2 just because... it's always positive and the results looked better than plain sine... ?
    s->codeAppendf("half ninety_degreesness = determinant(half2x2(%s, %s));", leftDir, rightDir);
    s->codeAppend ("ninety_degreesness = ninety_degreesness * ninety_degreesness;");

    // The below formula is not smart. It was just arrived at by considering the following
    // observations:
    //
    // 1. 90-degree, axis-aligned corners have full attenuation along the bisector.
    //    (i.e. coverage = 1 - distance_to_corner^2)
    //    (i.e. outputAttenuation = 0)
    //
    // 2. 180-degree corners always have zero attenuation.
    //    (i.e. coverage = 1 - distance_to_corner)
    //    (i.e. outputAttenuation = 1)
    //
    // 3. 90-degree corners whose bisector falls on a 45 degree angle also do not attenuate.
    //    (i.e. outputAttenuation = 1)
    s->codeAppendf("%s = max(obtuseness, axis_alignedness * ninety_degreesness);",
                   outputAttenuation);
}

void GrCCCoverageProcessor::getGLSLProcessorKey(const GrShaderCaps&,
                                                GrProcessorKeyBuilder* b) const {
    int key = (int)fPrimitiveType << 2;
    if (GSSubpass::kCorners == fGSSubpass) {
        key |= 2;
    }
    if (Impl::kVertexShader == fImpl) {
        key |= 1;
    }
#ifdef SK_DEBUG
    uint32_t bloatBits;
    memcpy(&bloatBits, &fDebugBloat, 4);
    b->add32(bloatBits);
#endif
    b->add32(key);
}

GrGLSLPrimitiveProcessor* GrCCCoverageProcessor::createGLSLInstance(const GrShaderCaps&) const {
    std::unique_ptr<Shader> shader;
    switch (fPrimitiveType) {
        case PrimitiveType::kTriangles:
        case PrimitiveType::kWeightedTriangles:
            shader = skstd::make_unique<TriangleShader>();
            break;
        case PrimitiveType::kQuadratics:
            shader = skstd::make_unique<GrCCQuadraticShader>();
            break;
        case PrimitiveType::kCubics:
            shader = skstd::make_unique<GrCCCubicShader>();
            break;
        case PrimitiveType::kConics:
            shader = skstd::make_unique<GrCCConicShader>();
            break;
    }
    return Impl::kGeometryShader == fImpl ? this->createGSImpl(std::move(shader))
                                          : this->createVSImpl(std::move(shader));
}

void GrCCCoverageProcessor::Shader::emitFragmentCode(const GrCCCoverageProcessor& proc,
                                                     GrGLSLFPFragmentBuilder* f,
                                                     const char* skOutputColor,
                                                     const char* skOutputCoverage) const {
    f->codeAppendf("half coverage = 0;");
    this->onEmitFragmentCode(f, "coverage");
    f->codeAppendf("%s.a = coverage;", skOutputColor);
    f->codeAppendf("%s = half4(1);", skOutputCoverage);
}

void GrCCCoverageProcessor::draw(GrOpFlushState* flushState, const GrPipeline& pipeline,
                                 const GrMesh meshes[],
                                 const GrPipeline::DynamicState dynamicStates[], int meshCount,
                                 const SkRect& drawBounds) const {
    GrGpuRTCommandBuffer* cmdBuff = flushState->rtCommandBuffer();
    cmdBuff->draw(pipeline, *this, meshes, dynamicStates, meshCount, drawBounds);

    // Geometry shader backend draws primitives in two subpasses.
    if (Impl::kGeometryShader == fImpl) {
        SkASSERT(GSSubpass::kHulls == fGSSubpass);
        GrCCCoverageProcessor cornerProc(*this, GSSubpass::kCorners);
        cmdBuff->draw(pipeline, cornerProc, meshes, dynamicStates, meshCount, drawBounds);
    }
}
