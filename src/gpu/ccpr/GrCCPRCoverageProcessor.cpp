/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRCoverageProcessor.h"

#include "ccpr/GrCCPRTriangleProcessor.h"
#include "ccpr/GrCCPRQuadraticProcessor.h"
#include "ccpr/GrCCPRCubicProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryShaderBuilder.h"
#include "glsl/GrGLSLProgramBuilder.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

const char* GrCCPRCoverageProcessor::GetProcessorName(Mode mode) {
    switch (mode) {
        case Mode::kTriangleHulls:
            return "GrCCPRTriangleHullAndEdgeProcessor (hulls)";
        case Mode::kTriangleEdges:
            return "GrCCPRTriangleHullAndEdgeProcessor (edges)";
        case Mode::kCombinedTriangleHullsAndEdges:
            return "GrCCPRTriangleHullAndEdgeProcessor (combined hulls & edges)";
        case Mode::kTriangleCorners:
            return "GrCCPRTriangleCornerProcessor";
        case Mode::kQuadraticHulls:
            return "GrCCPRQuadraticHullProcessor";
        case Mode::kQuadraticFlatEdges:
            return "GrCCPRQuadraticSharedEdgeProcessor";
        case Mode::kSerpentineInsets:
            return "GrCCPRCubicInsetProcessor (serpentine)";
        case Mode::kSerpentineBorders:
            return "GrCCPRCubicBorderProcessor (serpentine)";
        case Mode::kLoopInsets:
            return "GrCCPRCubicInsetProcessor (loop)";
        case Mode::kLoopBorders:
            return "GrCCPRCubicBorderProcessor (loop)";
    }
    SkFAIL("Unexpected ccpr coverage processor mode.");
    return nullptr;
}

GrCCPRCoverageProcessor::GrCCPRCoverageProcessor(Mode mode, GrBuffer* pointsBuffer)
        : fMode(mode)
        , fInstanceAttrib(this->addInstanceAttrib("instance", kVec4i_GrVertexAttribType,
                                                  kHigh_GrSLPrecision)) {
    fPointsBufferAccess.reset(kRG_float_GrPixelConfig, pointsBuffer, kVertex_GrShaderFlag);
    this->addBufferAccess(&fPointsBufferAccess);

    this->setWillUseGeoShader();

    this->initClassID<GrCCPRCoverageProcessor>();
}

void GrCCPRCoverageProcessor::getGLSLProcessorKey(const GrShaderCaps&,
                                                  GrProcessorKeyBuilder* b) const {
    b->add32(int(fMode));
}

GrGLSLPrimitiveProcessor* GrCCPRCoverageProcessor::createGLSLInstance(const GrShaderCaps&) const {
    switch (fMode) {
        using GeometryType = GrCCPRTriangleHullAndEdgeProcessor::GeometryType;

        case Mode::kTriangleHulls:
            return new GrCCPRTriangleHullAndEdgeProcessor(GeometryType::kHulls);
        case Mode::kTriangleEdges:
            return new GrCCPRTriangleHullAndEdgeProcessor(GeometryType::kEdges);
        case Mode::kCombinedTriangleHullsAndEdges:
            return new GrCCPRTriangleHullAndEdgeProcessor(GeometryType::kHullsAndEdges);
        case Mode::kTriangleCorners:
            return new GrCCPRTriangleCornerProcessor();
        case Mode::kQuadraticHulls:
            return new GrCCPRQuadraticHullProcessor();
        case Mode::kQuadraticFlatEdges:
            return new GrCCPRQuadraticSharedEdgeProcessor();
        case Mode::kSerpentineInsets:
            return new GrCCPRCubicInsetProcessor(GrCCPRCubicProcessor::Type::kSerpentine);
        case Mode::kSerpentineBorders:
            return new GrCCPRCubicBorderProcessor(GrCCPRCubicProcessor::Type::kSerpentine);
        case Mode::kLoopInsets:
            return new GrCCPRCubicInsetProcessor(GrCCPRCubicProcessor::Type::kLoop);
        case Mode::kLoopBorders:
            return new GrCCPRCubicBorderProcessor(GrCCPRCubicProcessor::Type::kLoop);
    }
    SkFAIL("Unexpected ccpr coverage processor mode.");
    return nullptr;
}

using PrimitiveProcessor = GrCCPRCoverageProcessor::PrimitiveProcessor;

void PrimitiveProcessor::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    const GrCCPRCoverageProcessor& proc = args.fGP.cast<GrCCPRCoverageProcessor>();

    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    switch (fCoverageType) {
        case CoverageType::kOne:
        case CoverageType::kShader:
            varyingHandler->addFlatVarying("wind", &fFragWind, kLow_GrSLPrecision);
            break;
        case CoverageType::kInterpolated:
            varyingHandler->addVarying("coverage_times_wind", &fFragCoverageTimesWind,
                                       kMedium_GrSLPrecision);
            break;
    }
    this->resetVaryings(varyingHandler);

    varyingHandler->emitAttributes(proc);

    this->emitVertexShader(proc, args.fVertBuilder, args.fTexelBuffers[0], args.fRTAdjustName,
                           gpArgs);
    this->emitGeometryShader(proc, args.fGeomBuilder, args.fRTAdjustName);
    this->emitCoverage(proc, args.fFragBuilder, args.fOutputColor, args.fOutputCoverage);

    SkASSERT(!args.fFPCoordTransformHandler->nextCoordTransform());
}

void PrimitiveProcessor::emitVertexShader(const GrCCPRCoverageProcessor& proc,
                                          GrGLSLVertexBuilder* v,
                                          const TexelBufferHandle& pointsBuffer,
                                          const char* rtAdjust, GrGPArgs* gpArgs) const {
    v->codeAppendf("int packedoffset = %s.w;", proc.instanceAttrib());
    v->codeAppend ("highp vec2 atlasoffset = vec2((packedoffset<<16) >> 16, packedoffset >> 16);");

    this->onEmitVertexShader(proc, v, pointsBuffer, "atlasoffset", rtAdjust, gpArgs);
}

void PrimitiveProcessor::emitGeometryShader(const GrCCPRCoverageProcessor& proc,
                                            GrGLSLGeometryBuilder* g, const char* rtAdjust) const {
    g->declareGlobal(fGeomWind);
    this->emitWind(g, rtAdjust, fGeomWind.c_str());

    SkString emitVertexFn;
    SkSTArray<2, GrShaderVar> emitArgs;
    const char* position = emitArgs.emplace_back("position", kVec2f_GrSLType,
                                                 GrShaderVar::kNonArray,
                                                 kHigh_GrSLPrecision).c_str();
    const char* coverage = emitArgs.emplace_back("coverage", kFloat_GrSLType,
                                                 GrShaderVar::kNonArray,
                                                 kHigh_GrSLPrecision).c_str();
    g->emitFunction(kVoid_GrSLType, "emitVertex", emitArgs.count(), emitArgs.begin(), [&]() {
        SkString fnBody;
        this->emitPerVertexGeometryCode(&fnBody, position, coverage, fGeomWind.c_str());
        if (fFragWind.gsOut()) {
            fnBody.appendf("%s = %s;", fFragWind.gsOut(), fGeomWind.c_str());
        }
        if (fFragCoverageTimesWind.gsOut()) {
            fnBody.appendf("%s = %s * %s;",
                           fFragCoverageTimesWind.gsOut(), coverage, fGeomWind.c_str());
        }
        fnBody.append ("gl_Position = vec4(position, 0, 1);");
        fnBody.append ("EmitVertex();");
        return fnBody;
    }().c_str(), &emitVertexFn);

    g->codeAppendf("highp vec2 bloat = %f * abs(%s.xz);", kAABloatRadius, rtAdjust);

#ifdef SK_DEBUG
    if (proc.debugVisualizations()) {
        g->codeAppendf("bloat *= %f;", GrCCPRCoverageProcessor::kDebugBloat);
    }
#endif

    return this->onEmitGeometryShader(g, emitVertexFn.c_str(), fGeomWind.c_str(), rtAdjust);
}

int PrimitiveProcessor::emitHullGeometry(GrGLSLGeometryBuilder* g, const char* emitVertexFn,
                                         const char* polygonPts, int numSides,
                                         const char* wedgeIdx, const char* insetPts) const {
    SkASSERT(numSides >= 3);

    if (!insetPts) {
        g->codeAppendf("highp vec2 centroidpt = %s * vec%i(%f);",
                       polygonPts, numSides, 1.0 / numSides);
    }

    g->codeAppendf("int previdx = (%s + %i) %% %i, "
                       "nextidx = (%s + 1) %% %i;",
                   wedgeIdx, numSides - 1, numSides, wedgeIdx, numSides);

    g->codeAppendf("highp vec2 self = %s[%s];"
                   "highp int leftidx = %s > 0 ? previdx : nextidx;"
                   "highp int rightidx = %s > 0 ? nextidx : previdx;",
                   polygonPts, wedgeIdx, fGeomWind.c_str(), fGeomWind.c_str());

    // Which quadrant does the vector from self -> right fall into?
    g->codeAppendf("highp vec2 right = %s[rightidx];", polygonPts);
    if (3 == numSides) {
        // TODO: evaluate perf gains.
        g->codeAppend ("highp vec2 qsr = sign(right - self);");
    } else {
        SkASSERT(4 == numSides);
        g->codeAppendf("highp vec2 diag = %s[(%s + 2) %% 4];", polygonPts, wedgeIdx);
        g->codeAppend ("highp vec2 qsr = sign((right != self ? right : diag) - self);");
    }

    // Which quadrant does the vector from left -> self fall into?
    g->codeAppendf("highp vec2 qls = sign(self - %s[leftidx]);", polygonPts);

    // d2 just helps us reduce triangle counts with orthogonal, axis-aligned lines.
    // TODO: evaluate perf gains.
    const char* dr2 = "dr";
    if (3 == numSides) {
        // TODO: evaluate perf gains.
        g->codeAppend ("highp vec2 dr = vec2(qsr.y != 0 ? +qsr.y : +qsr.x, "
                                            "qsr.x != 0 ? -qsr.x : +qsr.y);");
        g->codeAppend ("highp vec2 dr2 = vec2(qsr.y != 0 ? +qsr.y : -qsr.x, "
                                             "qsr.x != 0 ? -qsr.x : -qsr.y);");
        g->codeAppend ("highp vec2 dl = vec2(qls.y != 0 ? +qls.y : +qls.x, "
                                            "qls.x != 0 ? -qls.x : +qls.y);");
        dr2 = "dr2";
    } else {
        g->codeAppend ("highp vec2 dr = vec2(qsr.y != 0 ? +qsr.y : 1, "
                                            "qsr.x != 0 ? -qsr.x : 1);");
        g->codeAppend ("highp vec2 dl = (qls == vec2(0)) ? dr : vec2(qls.y != 0 ? +qls.y : 1, "
                                                                    "qls.x != 0 ? -qls.x : 1);");
    }
    g->codeAppendf("bvec2 dnotequal = notEqual(%s, dl);", dr2);

    // Emit one third of what is the convex hull of pixel-size boxes centered on the vertices.
    // Each invocation emits a different third.
    if (insetPts) {
        g->codeAppendf("%s(%s[rightidx], 1);", emitVertexFn, insetPts);
    }
    g->codeAppendf("%s(right + bloat * dr, 1);", emitVertexFn);
    if (insetPts) {
        g->codeAppendf("%s(%s[%s], 1);", emitVertexFn, insetPts, wedgeIdx);
    } else {
        g->codeAppendf("%s(centroidpt, 1);", emitVertexFn);
    }
    g->codeAppendf("%s(self + bloat * %s, 1);", emitVertexFn, dr2);
    g->codeAppend ("if (any(dnotequal)) {");
    g->codeAppendf(    "%s(self + bloat * dl, 1);", emitVertexFn);
    g->codeAppend ("}");
    g->codeAppend ("if (all(dnotequal)) {");
    g->codeAppendf(    "%s(self + bloat * vec2(-dl.y, dl.x), 1);", emitVertexFn);
    g->codeAppend ("}");
    g->codeAppend ("EndPrimitive();");

    return insetPts ? 6 : 5;
}

int PrimitiveProcessor::emitEdgeGeometry(GrGLSLGeometryBuilder* g, const char* emitVertexFn,
                                         const char* leftPt, const char* rightPt,
                                         const char* distanceEquation) const {
    if (!distanceEquation) {
        this->emitEdgeDistanceEquation(g, leftPt, rightPt, "highp vec3 edge_distance_equation");
        distanceEquation = "edge_distance_equation";
    }

    // qlr is defined in emitEdgeDistanceEquation.
    g->codeAppendf("highp mat2 endpts = mat2(%s - bloat * qlr, %s + bloat * qlr);",
                   leftPt, rightPt);
    g->codeAppendf("mediump vec2 endpts_coverage = %s.xy * endpts + %s.z;",
                   distanceEquation, distanceEquation);

    // d1 is defined in emitEdgeDistanceEquation.
    g->codeAppend ("highp vec2 d2 = d1;");
    g->codeAppend ("bool aligned = qlr.x == 0 || qlr.y == 0;");
    g->codeAppend ("if (aligned) {");
    g->codeAppend (    "d1 -= qlr;");
    g->codeAppend (    "d2 += qlr;");
    g->codeAppend ("}");

    // Emit the convex hull of 2 pixel-size boxes centered on the endpoints of the edge. Each
    // invocation emits a different edge. Emit negative coverage that subtracts the appropiate
    // amount back out from the hull we drew above.
    g->codeAppend ("if (!aligned) {");
    g->codeAppendf(    "%s(endpts[0], endpts_coverage[0]);", emitVertexFn);
    g->codeAppend ("}");
    g->codeAppendf("%s(%s + bloat * d1, -1);", emitVertexFn, leftPt);
    g->codeAppendf("%s(%s - bloat * d2, 0);", emitVertexFn, leftPt);
    g->codeAppendf("%s(%s + bloat * d2, -1);", emitVertexFn, rightPt);
    g->codeAppendf("%s(%s - bloat * d1, 0);", emitVertexFn, rightPt);
    g->codeAppend ("if (!aligned) {");
    g->codeAppendf(    "%s(endpts[1], endpts_coverage[1]);", emitVertexFn);
    g->codeAppend ("}");
    g->codeAppend ("EndPrimitive();");

    return 6;
}

void PrimitiveProcessor::emitEdgeDistanceEquation(GrGLSLGeometryBuilder* g,
                                                  const char* leftPt, const char* rightPt,
                                                  const char* outputDistanceEquation) const {
    // Which quadrant does the vector from left -> right fall into?
    g->codeAppendf("highp vec2 qlr = sign(%s - %s);", rightPt, leftPt);
    g->codeAppend ("highp vec2 d1 = vec2(qlr.y, -qlr.x);");

    g->codeAppendf("highp vec2 n = vec2(%s.y - %s.y, %s.x - %s.x);",
                   rightPt, leftPt, leftPt, rightPt);
    g->codeAppendf("highp vec2 kk = n * mat2(%s + bloat * d1, %s - bloat * d1);", leftPt, leftPt);
    // Clamp for when n=0. wind=0 when n=0 so as long as we don't get Inf or NaN we are fine.
    g->codeAppendf("highp float scale = 1 / max(kk[0] - kk[1], 1e-30);");

    g->codeAppendf("%s = vec3(-n, kk[1]) * scale;", outputDistanceEquation);
}

void PrimitiveProcessor::emitCoverage(const GrCCPRCoverageProcessor& proc, GrGLSLFragmentBuilder* f,
                                      const char* outputColor, const char* outputCoverage) const {
    switch (fCoverageType) {
        case CoverageType::kOne:
            f->codeAppendf("%s.a = %s;", outputColor, fFragWind.fsIn());
            break;
        case CoverageType::kInterpolated:
            f->codeAppendf("%s.a = %s;", outputColor, fFragCoverageTimesWind.fsIn());
            break;
        case CoverageType::kShader:
            f->codeAppendf("mediump float coverage = 0;");
            this->emitShaderCoverage(f, "coverage");
            f->codeAppendf("%s.a = coverage * %s;", outputColor, fFragWind.fsIn());
            break;
    }

    f->codeAppendf("%s = vec4(1);", outputCoverage);

#ifdef SK_DEBUG
    if (proc.debugVisualizations()) {
        f->codeAppendf("%s = vec4(-%s.a, %s.a, 0, 1);", outputColor, outputColor, outputColor);
    }
#endif
}

int PrimitiveProcessor::defineSoftSampleLocations(GrGLSLFragmentBuilder* f,
                                                  const char* samplesName) const {
    // Standard DX11 sample locations.
#if defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_IOS)
    f->defineConstant("highp vec2[8]", samplesName, "vec2[8]("
        "vec2(+1, -3)/16, vec2(-1, +3)/16, vec2(+5, +1)/16, vec2(-3, -5)/16, "
        "vec2(-5, +5)/16, vec2(-7, -1)/16, vec2(+3, +7)/16, vec2(+7, -7)/16."
    ")");
    return 8;
#else
    f->defineConstant("highp vec2[16]", samplesName, "vec2[16]("
        "vec2(+1, +1)/16, vec2(-1, -3)/16, vec2(-3, +2)/16, vec2(+4, -1)/16, "
        "vec2(-5, -2)/16, vec2(+2, +5)/16, vec2(+5, +3)/16, vec2(+3, -5)/16, "
        "vec2(-2, +6)/16, vec2( 0, -7)/16, vec2(-4, -6)/16, vec2(-6, +4)/16, "
        "vec2(-8,  0)/16, vec2(+7, -4)/16, vec2(+6, +7)/16, vec2(-7, -8)/16."
    ")");
    return 16;
#endif
}

#ifdef SK_DEBUG

#include "GrRenderTarget.h"

void GrCCPRCoverageProcessor::Validate(GrRenderTarget* atlasTexture) {
    SkASSERT(kAtlasOrigin == atlasTexture->origin());
    SkASSERT(GrPixelConfigIsAlphaOnly(atlasTexture->config()));
    SkASSERT(GrPixelConfigIsFloatingPoint(atlasTexture->config()));
}

#endif
