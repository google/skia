/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCStroker.h"

#include "include/core/SkStrokeRec.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/ccpr/GrCCCoverageProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

static constexpr int kMaxNumLinearSegmentsLog2 = GrCCStrokeGeometry::kMaxNumLinearSegmentsLog2;
using TriangleInstance = GrCCCoverageProcessor::TriPointInstance;
using ConicInstance = GrCCCoverageProcessor::QuadPointInstance;

namespace {

struct LinearStrokeInstance {
    float fEndpoints[4];
    float fStrokeRadius;

    inline void set(const SkPoint[2], float dx, float dy, float strokeRadius);
};

inline void LinearStrokeInstance::set(const SkPoint P[2], float dx, float dy, float strokeRadius) {
    Sk2f X, Y;
    Sk2f::Load2(P, &X, &Y);
    Sk2f::Store2(fEndpoints, X + dx, Y + dy);
    fStrokeRadius = strokeRadius;
}

struct CubicStrokeInstance {
    float fX[4];
    float fY[4];
    float fStrokeRadius;
    float fNumSegments;

    inline void set(const SkPoint[4], float dx, float dy, float strokeRadius, int numSegments);
    inline void set(const Sk4f& X, const Sk4f& Y, float dx, float dy, float strokeRadius,
                    int numSegments);
};

inline void CubicStrokeInstance::set(const SkPoint P[4], float dx, float dy, float strokeRadius,
                                     int numSegments) {
    Sk4f X, Y;
    Sk4f::Load2(P, &X, &Y);
    this->set(X, Y, dx, dy, strokeRadius, numSegments);
}

inline void CubicStrokeInstance::set(const Sk4f& X, const Sk4f& Y, float dx, float dy,
                                     float strokeRadius, int numSegments) {
    (X + dx).store(&fX);
    (Y + dy).store(&fY);
    fStrokeRadius = strokeRadius;
    fNumSegments = static_cast<float>(numSegments);
}

// This class draws stroked lines in post-transform device space (a.k.a. rectangles). Rigid-body
// transforms can be achieved by transforming the line ahead of time and adjusting the stroke
// width. Skews of the stroke itself are not yet supported.
//
// Corner coverage is AA-correct, meaning, n^2 attenuation along the diagonals. This is important
// for seamless integration with the connecting geometry.
class LinearStrokeProcessor : public GrGeometryProcessor {
public:
    LinearStrokeProcessor() : GrGeometryProcessor(kLinearStrokeProcessor_ClassID) {
        this->setInstanceAttributes(kInstanceAttribs, 2);
#ifdef SK_DEBUG
        using Instance = LinearStrokeInstance;
        SkASSERT(this->instanceStride() == sizeof(Instance));
#endif
    }

private:
    const char* name() const override { return "LinearStrokeProcessor"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    static constexpr Attribute kInstanceAttribs[2] = {
            {"endpts", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"stroke_radius", kFloat_GrVertexAttribType, kFloat_GrSLType}
    };

    class Impl : public GrGLSLGeometryProcessor {
        void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&,
                     FPCoordTransformIter&&) override {}
        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override;
    };

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new Impl();
    }
};

void LinearStrokeProcessor::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLUniformHandler* uniHandler = args.fUniformHandler;

    varyingHandler->emitAttributes(args.fGP.cast<LinearStrokeProcessor>());

    GrGLSLVertexBuilder* v = args.fVertBuilder;
    v->codeAppend ("float2 tan = normalize(endpts.zw - endpts.xy);");
    v->codeAppend ("float2 n = float2(tan.y, -tan.x);");
    v->codeAppend ("float nwidth = abs(n.x) + abs(n.y);");

    // Outset the vertex position for AA butt caps.
    v->codeAppend ("float2 outset = tan*nwidth/2;");
    v->codeAppend ("float2 position = (sk_VertexID < 2) "
                           "? endpts.xy - outset : endpts.zw + outset;");

    // Calculate Manhattan distance from both butt caps, where distance=0 on the actual endpoint and
    // distance=-.5 on the outset edge.
    GrGLSLVarying edgeDistances(kFloat4_GrSLType);
    varyingHandler->addVarying("edge_distances", &edgeDistances);
    v->codeAppendf("%s.xz = float2(-.5, dot(endpts.zw - endpts.xy, tan) / nwidth + .5);",
                   edgeDistances.vsOut());
    v->codeAppendf("%s.xz = (sk_VertexID < 2) ? %s.xz : %s.zx;",
                   edgeDistances.vsOut(), edgeDistances.vsOut(), edgeDistances.vsOut());

    // Outset the vertex position for stroke radius plus edge AA.
    v->codeAppend ("outset = n * (stroke_radius + nwidth/2);");
    v->codeAppend ("position += (0 == (sk_VertexID & 1)) ? +outset : -outset;");

    // Calculate Manhattan distance from both edges, where distance=0 on the actual edge and
    // distance=-.5 on the outset.
    v->codeAppendf("%s.yw = float2(-.5, 2*stroke_radius / nwidth + .5);", edgeDistances.vsOut());
    v->codeAppendf("%s.yw = (0 == (sk_VertexID & 1)) ? %s.yw : %s.wy;",
                   edgeDistances.vsOut(), edgeDistances.vsOut(), edgeDistances.vsOut());

    gpArgs->fPositionVar.set(kFloat2_GrSLType, "position");
    this->emitTransforms(v, varyingHandler, uniHandler, GrShaderVar("position", kFloat2_GrSLType),
                         SkMatrix::I(), args.fFPCoordTransformHandler);

    // Use the 4 edge distances to calculate coverage in the fragment shader.
    GrGLSLFPFragmentBuilder* f = args.fFragBuilder;
    f->codeAppendf("half2 coverages = half2(min(%s.xy, .5) + min(%s.zw, .5));",
                   edgeDistances.fsIn(), edgeDistances.fsIn());
    f->codeAppendf("%s = half4(coverages.x * coverages.y);", args.fOutputColor);

    // This shader doesn't use the built-in Ganesh coverage.
    f->codeAppendf("%s = half4(1);", args.fOutputCoverage);
}

constexpr GrPrimitiveProcessor::Attribute LinearStrokeProcessor::kInstanceAttribs[];

// This class draws stroked cubics in post-transform device space. Rigid-body transforms can be
// achieved by transforming the curve ahead of time and adjusting the stroke width. Skews of the
// stroke itself are not yet supported. Quadratics can be drawn by converting them to cubics.
//
// This class works by finding stroke-width line segments orthogonal to the curve at a
// pre-determined number of evenly spaced points along the curve (evenly spaced in the parametric
// sense). It then connects the segments with a triangle strip. As for common in CCPR, clockwise-
// winding triangles from the strip emit positive coverage, counter-clockwise triangles emit
// negative, and we use SkBlendMode::kPlus.
class CubicStrokeProcessor : public GrGeometryProcessor {
public:
    CubicStrokeProcessor() : GrGeometryProcessor(kCubicStrokeProcessor_ClassID) {
        this->setInstanceAttributes(kInstanceAttribs, 3);
#ifdef SK_DEBUG
        using Instance = CubicStrokeInstance;
        SkASSERT(this->instanceStride() == sizeof(Instance));
#endif
    }

private:
    const char* name() const override { return "CubicStrokeProcessor"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    static constexpr Attribute kInstanceAttribs[3] = {
            {"X", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"Y", kFloat4_GrVertexAttribType, kFloat4_GrSLType},
            {"stroke_info", kFloat2_GrVertexAttribType, kFloat2_GrSLType}
    };

    class Impl : public GrGLSLGeometryProcessor {
        void setData(const GrGLSLProgramDataManager&, const GrPrimitiveProcessor&,
                     FPCoordTransformIter&&) override {}
        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override;
    };

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new Impl();
    }
};

void CubicStrokeProcessor::Impl::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLUniformHandler* uniHandler = args.fUniformHandler;

    varyingHandler->emitAttributes(args.fGP.cast<CubicStrokeProcessor>());

    GrGLSLVertexBuilder* v = args.fVertBuilder;
    v->codeAppend ("float4x2 P = transpose(float2x4(X, Y));");
    v->codeAppend ("float stroke_radius = stroke_info[0];");
    v->codeAppend ("float num_segments = stroke_info[1];");

    // Find the parametric T value at which we will emit our orthogonal line segment. We emit two
    // line segments at T=0 and double at T=1 as well for AA butt caps.
    v->codeAppend ("float point_id = float(sk_VertexID/2);");
    v->codeAppend ("float T = max((point_id - 1) / num_segments, 0);");
    v->codeAppend ("T = (point_id >= num_segments + 1) ? 1 : T;");  // In case x/x !== 1.

    // Use De Casteljau's algorithm to find the position and tangent for our orthogonal line
    // segment. De Casteljau's is more numerically stable than evaluating the curve and derivative
    // directly.
    v->codeAppend ("float2 ab = mix(P[0], P[1], T);");
    v->codeAppend ("float2 bc = mix(P[1], P[2], T);");
    v->codeAppend ("float2 cd = mix(P[2], P[3], T);");
    v->codeAppend ("float2 abc = mix(ab, bc, T);");
    v->codeAppend ("float2 bcd = mix(bc, cd, T);");
    v->codeAppend ("float2 position = mix(abc, bcd, T);");
    v->codeAppend ("float2 tan = bcd - abc;");

    // Find actual tangents for the corner cases when De Casteljau's yields tan=0. (We shouldn't
    // encounter other numerically unstable cases where tan ~= 0, because GrCCStrokeGeometry snaps
    // control points to endpoints in curves where they are almost equal.)
    v->codeAppend ("if (0 == T && P[0] == P[1]) {");
    v->codeAppend (    "tan = P[2] - P[0];");
    v->codeAppend ("}");
    v->codeAppend ("if (1 == T && P[2] == P[3]) {");
    v->codeAppend (    "tan = P[3] - P[1];");
    v->codeAppend ("}");
    v->codeAppend ("tan = normalize(tan);");
    v->codeAppend ("float2 n = float2(tan.y, -tan.x);");
    v->codeAppend ("float nwidth = abs(n.x) + abs(n.y);");

    // Outset the vertex position for stroke radius plus edge AA.
    v->codeAppend ("float2 outset = n * (stroke_radius + nwidth/2);");
    v->codeAppend ("position += (0 == (sk_VertexID & 1)) ? -outset : +outset;");

    // Calculate the Manhattan distance from both edges, where distance=0 on the actual edge and
    // distance=-.5 on the outset.
    GrGLSLVarying coverages(kFloat3_GrSLType);
    varyingHandler->addVarying("coverages", &coverages);
    v->codeAppendf("%s.xy = float2(-.5, 2*stroke_radius / nwidth + .5);", coverages.vsOut());
    v->codeAppendf("%s.xy = (0 == (sk_VertexID & 1)) ? %s.xy : %s.yx;",
                   coverages.vsOut(), coverages.vsOut(), coverages.vsOut());

    // Adjust the orthogonal line segments on the endpoints so they straddle the actual endpoint
    // at a Manhattan distance of .5 on either side.
    v->codeAppend ("if (0 == point_id || num_segments+1 == point_id) {");
    v->codeAppend (    "position -= tan*nwidth/2;");
    v->codeAppend ("}");
    v->codeAppend ("if (1 == point_id || num_segments+2 == point_id) {");
    v->codeAppend (    "position += tan*nwidth/2;");
    v->codeAppend ("}");

    // Interpolate coverage for butt cap AA from 0 on the outer segment to 1 on the inner.
    v->codeAppendf("%s.z = (0 == point_id || num_segments+2 == point_id) ? 0 : 1;",
                   coverages.vsOut());

    gpArgs->fPositionVar.set(kFloat2_GrSLType, "position");
    this->emitTransforms(v, varyingHandler, uniHandler, GrShaderVar("position", kFloat2_GrSLType),
                         SkMatrix::I(), args.fFPCoordTransformHandler);

    // Use the 2 edge distances and interpolated butt cap AA to calculate fragment coverage.
    GrGLSLFPFragmentBuilder* f = args.fFragBuilder;
    f->codeAppendf("half2 edge_coverages = min(half2(%s.xy), .5);", coverages.fsIn());
    f->codeAppend ("half coverage = edge_coverages.x + edge_coverages.y;");
    f->codeAppendf("coverage *= half(%s.z);", coverages.fsIn());  // Butt cap AA.

    // As is common for CCPR, clockwise-winding triangles from the strip emit positive coverage, and
    // counter-clockwise triangles emit negative.
    f->codeAppendf("%s = half4(sk_Clockwise ? +coverage : -coverage);", args.fOutputColor);

    // This shader doesn't use the built-in Ganesh coverage.
    f->codeAppendf("%s = half4(1);", args.fOutputCoverage);
}

constexpr GrPrimitiveProcessor::Attribute CubicStrokeProcessor::kInstanceAttribs[];

}  // anonymous namespace

void GrCCStroker::parseDeviceSpaceStroke(const SkPath& path, const SkPoint* deviceSpacePts,
                                         const SkStrokeRec& stroke, float strokeDevWidth,
                                         GrScissorTest scissorTest,
                                         const SkIRect& clippedDevIBounds,
                                         const SkIVector& devToAtlasOffset) {
    SkASSERT(SkStrokeRec::kStroke_Style == stroke.getStyle() ||
             SkStrokeRec::kHairline_Style == stroke.getStyle());
    SkASSERT(!fInstanceBuffer);
    SkASSERT(!path.isEmpty());

    if (!fHasOpenBatch) {
        fBatches.emplace_back(&fTalliesAllocator, *fInstanceCounts[(int)GrScissorTest::kDisabled],
                              fScissorSubBatches.count());
        fInstanceCounts[(int)GrScissorTest::kDisabled] = fBatches.back().fNonScissorEndInstances;
        fHasOpenBatch = true;
    }

    InstanceTallies* currStrokeEndIndices;
    if (GrScissorTest::kEnabled == scissorTest) {
        SkASSERT(fBatches.back().fEndScissorSubBatch == fScissorSubBatches.count());
        fScissorSubBatches.emplace_back(
                &fTalliesAllocator, *fInstanceCounts[(int)GrScissorTest::kEnabled],
                clippedDevIBounds.makeOffset(devToAtlasOffset.x(), devToAtlasOffset.y()));
        fBatches.back().fEndScissorSubBatch = fScissorSubBatches.count();
        fInstanceCounts[(int)GrScissorTest::kEnabled] =
                currStrokeEndIndices = fScissorSubBatches.back().fEndInstances;
    } else {
        currStrokeEndIndices = fBatches.back().fNonScissorEndInstances;
    }

    fGeometry.beginPath(stroke, strokeDevWidth, currStrokeEndIndices);

    fPathInfos.push_back() = {devToAtlasOffset, strokeDevWidth/2, scissorTest};

    int devPtsIdx = 0;
    SkPath::Verb previousVerb = SkPath::kClose_Verb;

    for (SkPath::Verb verb : SkPathPriv::Verbs(path)) {
        SkASSERT(SkPath::kDone_Verb != previousVerb);
        const SkPoint* P = &deviceSpacePts[devPtsIdx - 1];
        switch (verb) {
            case SkPath::kMove_Verb:
                if (devPtsIdx > 0 && SkPath::kClose_Verb != previousVerb) {
                    fGeometry.capContourAndExit();
                }
                fGeometry.moveTo(deviceSpacePts[devPtsIdx]);
                ++devPtsIdx;
                break;
            case SkPath::kClose_Verb:
                SkASSERT(SkPath::kClose_Verb != previousVerb);
                fGeometry.closeContour();
                break;
            case SkPath::kLine_Verb:
                SkASSERT(SkPath::kClose_Verb != previousVerb);
                fGeometry.lineTo(P[1]);
                ++devPtsIdx;
                break;
            case SkPath::kQuad_Verb:
                SkASSERT(SkPath::kClose_Verb != previousVerb);
                fGeometry.quadraticTo(P);
                devPtsIdx += 2;
                break;
            case SkPath::kCubic_Verb: {
                SkASSERT(SkPath::kClose_Verb != previousVerb);
                fGeometry.cubicTo(P);
                devPtsIdx += 3;
                break;
            }
            case SkPath::kConic_Verb:
                SkASSERT(SkPath::kClose_Verb != previousVerb);
                SK_ABORT("Stroked conics not supported.");
                break;
            case SkPath::kDone_Verb:
                break;
        }
        previousVerb = verb;
    }

    if (devPtsIdx > 0 && SkPath::kClose_Verb != previousVerb) {
        fGeometry.capContourAndExit();
    }
}

// This class encapsulates the process of expanding ready-to-draw geometry from GrCCStrokeGeometry
// directly into GPU instance buffers.
class GrCCStroker::InstanceBufferBuilder {
public:
    InstanceBufferBuilder(GrOnFlushResourceProvider* onFlushRP, GrCCStroker* stroker) {
        memcpy(fNextInstances, stroker->fBaseInstances, sizeof(fNextInstances));
#ifdef SK_DEBUG
        fEndInstances[0] = stroker->fBaseInstances[0] + *stroker->fInstanceCounts[0];
        fEndInstances[1] = stroker->fBaseInstances[1] + *stroker->fInstanceCounts[1];
#endif

        int endConicsIdx = stroker->fBaseInstances[1].fConics +
                           stroker->fInstanceCounts[1]->fConics;
        fInstanceBuffer = onFlushRP->makeBuffer(GrGpuBufferType::kVertex,
                                                endConicsIdx * sizeof(ConicInstance));
        if (!fInstanceBuffer) {
            SkDebugf("WARNING: failed to allocate CCPR stroke instance buffer.\n");
            return;
        }
        fInstanceBufferData = fInstanceBuffer->map();
    }

    bool isMapped() const { return SkToBool(fInstanceBufferData); }

    void updateCurrentInfo(const PathInfo& pathInfo) {
        SkASSERT(this->isMapped());
        fCurrDX = static_cast<float>(pathInfo.fDevToAtlasOffset.x());
        fCurrDY = static_cast<float>(pathInfo.fDevToAtlasOffset.y());
        fCurrStrokeRadius = pathInfo.fStrokeRadius;
        fCurrNextInstances = &fNextInstances[(int)pathInfo.fScissorTest];
        SkDEBUGCODE(fCurrEndInstances = &fEndInstances[(int)pathInfo.fScissorTest]);
    }

    void appendLinearStroke(const SkPoint endpts[2]) {
        SkASSERT(this->isMapped());
        this->appendLinearStrokeInstance().set(endpts, fCurrDX, fCurrDY, fCurrStrokeRadius);
    }

    void appendQuadraticStroke(const SkPoint P[3], int numLinearSegmentsLog2) {
        SkASSERT(this->isMapped());
        SkASSERT(numLinearSegmentsLog2 > 0);

        Sk4f ptsT[2];
        Sk2f p0 = Sk2f::Load(P);
        Sk2f p1 = Sk2f::Load(P+1);
        Sk2f p2 = Sk2f::Load(P+2);

        // Convert the quadratic to cubic.
        Sk2f c1 = SkNx_fma(Sk2f(2/3.f), p1 - p0, p0);
        Sk2f c2 = SkNx_fma(Sk2f(1/3.f), p2 - p1, p1);
        Sk2f::Store4(ptsT, p0, c1, c2, p2);

        this->appendCubicStrokeInstance(numLinearSegmentsLog2).set(
                ptsT[0], ptsT[1], fCurrDX, fCurrDY, fCurrStrokeRadius, 1 << numLinearSegmentsLog2);
    }

    void appendCubicStroke(const SkPoint P[3], int numLinearSegmentsLog2) {
        SkASSERT(this->isMapped());
        SkASSERT(numLinearSegmentsLog2 > 0);
        this->appendCubicStrokeInstance(numLinearSegmentsLog2).set(
                P, fCurrDX, fCurrDY, fCurrStrokeRadius, 1 << numLinearSegmentsLog2);
    }

    void appendJoin(Verb joinVerb, const SkPoint& center, const SkVector& leftNorm,
                    const SkVector& rightNorm, float miterCapHeightOverWidth, float conicWeight) {
        SkASSERT(this->isMapped());

        Sk2f offset = Sk2f::Load(&center) + Sk2f(fCurrDX, fCurrDY);
        Sk2f n0 = Sk2f::Load(&leftNorm);
        Sk2f n1 = Sk2f::Load(&rightNorm);

        // Identify the outer edge.
        Sk2f cross = n0 * SkNx_shuffle<1,0>(n1);
        if (cross[0] < cross[1]) {
            Sk2f tmp = n0;
            n0 = -n1;
            n1 = -tmp;
        }

        if (!GrCCStrokeGeometry::IsInternalJoinVerb(joinVerb)) {
            // Normal joins are a triangle that connects the outer corners of two adjoining strokes.
            this->appendTriangleInstance().set(
                    n1 * fCurrStrokeRadius, Sk2f(0, 0), n0 * fCurrStrokeRadius, offset,
                    TriangleInstance::Ordering::kXYTransposed);
            if (Verb::kBevelJoin == joinVerb) {
                return;
            }
        } else {
            // Internal joins are coverage-counted, self-intersecting quadrilaterals that tie the
            // four corners of two adjoining strokes together a like a shoelace. Coverage is
            // negative on the inside half. We implement this geometry with a pair of triangles.
            this->appendTriangleInstance().set(
                    -n0 * fCurrStrokeRadius, n0 * fCurrStrokeRadius, n1 * fCurrStrokeRadius,
                    offset, TriangleInstance::Ordering::kXYTransposed);
            if (Verb::kBevelJoin == joinVerb) {
                return;
            }
            this->appendTriangleInstance().set(
                    -n0 * fCurrStrokeRadius, n1 * fCurrStrokeRadius, -n1 * fCurrStrokeRadius,
                    offset, TriangleInstance::Ordering::kXYTransposed);
            if (Verb::kBevelJoin == joinVerb) {
                return;
            }
            if (Verb::kInternalBevelJoin == joinVerb) {
                return;
            }
        }

        // For miter and round joins, we place an additional triangle cap on top of the bevel. This
        // triangle is literal for miters and is conic control points for round joins.
        SkASSERT(miterCapHeightOverWidth >= 0 || SkScalarIsNaN(miterCapHeightOverWidth));
        Sk2f base = n1 - n0;
        Sk2f baseNorm = Sk2f(base[1], -base[0]);
        Sk2f c = (n0 + n1) * .5f + baseNorm * miterCapHeightOverWidth;

        if (Verb::kMiterJoin == joinVerb) {
            this->appendTriangleInstance().set(
                    n0 * fCurrStrokeRadius, c * fCurrStrokeRadius, n1 * fCurrStrokeRadius, offset,
                    TriangleInstance::Ordering::kXYTransposed);
        } else {
            SkASSERT(Verb::kRoundJoin == joinVerb || Verb::kInternalRoundJoin == joinVerb);
            this->appendConicInstance().setW(n0 * fCurrStrokeRadius, c * fCurrStrokeRadius,
                                             n1 * fCurrStrokeRadius, offset, conicWeight);
            if (Verb::kInternalRoundJoin == joinVerb) {
                this->appendConicInstance().setW(-n1 * fCurrStrokeRadius, c * -fCurrStrokeRadius,
                                                 -n0 * fCurrStrokeRadius, offset, conicWeight);
            }
        }
    }

    void appendCap(Verb capType, const SkPoint& pt, const SkVector& norm) {
        SkASSERT(this->isMapped());

        Sk2f n = Sk2f::Load(&norm) * fCurrStrokeRadius;
        Sk2f v = Sk2f(-n[1], n[0]);
        Sk2f offset = Sk2f::Load(&pt) + Sk2f(fCurrDX, fCurrDY);

        if (Verb::kSquareCap == capType) {
            SkPoint endPts[2] = {{0, 0}, {v[0], v[1]}};
            this->appendLinearStrokeInstance().set(endPts, offset[0], offset[1], fCurrStrokeRadius);
        } else {
            SkASSERT(Verb::kRoundCap == capType);
            this->appendTriangleInstance().set(
                    n, v, -n, offset, TriangleInstance::Ordering::kXYTransposed);
            this->appendConicInstance().setW(n, n + v, v, offset, SK_ScalarRoot2Over2);
            this->appendConicInstance().setW(v, v - n, -n, offset, SK_ScalarRoot2Over2);
        }
    }

    sk_sp<GrGpuBuffer> finish() {
        SkASSERT(this->isMapped());
        SkASSERT(!memcmp(fNextInstances, fEndInstances, sizeof(fNextInstances)));
        fInstanceBuffer->unmap();
        fInstanceBufferData = nullptr;
        SkASSERT(!this->isMapped());
        return std::move(fInstanceBuffer);
    }

private:
    LinearStrokeInstance& appendLinearStrokeInstance() {
        int instanceIdx = fCurrNextInstances->fStrokes[0]++;
        SkASSERT(instanceIdx < fCurrEndInstances->fStrokes[0]);

        return reinterpret_cast<LinearStrokeInstance*>(fInstanceBufferData)[instanceIdx];
    }

    CubicStrokeInstance& appendCubicStrokeInstance(int numLinearSegmentsLog2) {
        SkASSERT(numLinearSegmentsLog2 > 0);
        SkASSERT(numLinearSegmentsLog2 <= kMaxNumLinearSegmentsLog2);

        int instanceIdx = fCurrNextInstances->fStrokes[numLinearSegmentsLog2]++;
        SkASSERT(instanceIdx < fCurrEndInstances->fStrokes[numLinearSegmentsLog2]);

        return reinterpret_cast<CubicStrokeInstance*>(fInstanceBufferData)[instanceIdx];
    }

    TriangleInstance& appendTriangleInstance() {
        int instanceIdx = fCurrNextInstances->fTriangles++;
        SkASSERT(instanceIdx < fCurrEndInstances->fTriangles);

        return reinterpret_cast<TriangleInstance*>(fInstanceBufferData)[instanceIdx];
    }

    ConicInstance& appendConicInstance() {
        int instanceIdx = fCurrNextInstances->fConics++;
        SkASSERT(instanceIdx < fCurrEndInstances->fConics);

        return reinterpret_cast<ConicInstance*>(fInstanceBufferData)[instanceIdx];
    }

    float fCurrDX, fCurrDY;
    float fCurrStrokeRadius;
    InstanceTallies* fCurrNextInstances;
    SkDEBUGCODE(const InstanceTallies* fCurrEndInstances);

    sk_sp<GrGpuBuffer> fInstanceBuffer;
    void* fInstanceBufferData = nullptr;
    InstanceTallies fNextInstances[2];
    SkDEBUGCODE(InstanceTallies fEndInstances[2]);
};

GrCCStroker::BatchID GrCCStroker::closeCurrentBatch() {
    if (!fHasOpenBatch) {
        return kEmptyBatchID;
    }
    int start = (fBatches.count() < 2) ? 0 : fBatches[fBatches.count() - 2].fEndScissorSubBatch;
    int end = fBatches.back().fEndScissorSubBatch;
    fMaxNumScissorSubBatches = SkTMax(fMaxNumScissorSubBatches, end - start);
    fHasOpenBatch = false;
    return fBatches.count() - 1;
}

bool GrCCStroker::prepareToDraw(GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(!fInstanceBuffer);
    SkASSERT(!fHasOpenBatch);  // Call closeCurrentBatch() first.

    // Here we layout a single instance buffer to share with every internal batch.
    //
    // Rather than place each instance array in its own GPU buffer, we allocate a single
    // megabuffer and lay them all out side-by-side. We can offset the "baseInstance" parameter in
    // our draw calls to direct the GPU to the applicable elements within a given array.
    fBaseInstances[0].fStrokes[0] = 0;
    fBaseInstances[1].fStrokes[0] = fInstanceCounts[0]->fStrokes[0];
    int endLinearStrokesIdx = fBaseInstances[1].fStrokes[0] + fInstanceCounts[1]->fStrokes[0];

    int cubicStrokesIdx = GrSizeDivRoundUp(endLinearStrokesIdx * sizeof(LinearStrokeInstance),
                                           sizeof(CubicStrokeInstance));
    for (int i = 1; i <= kMaxNumLinearSegmentsLog2; ++i) {
        for (int j = 0; j < kNumScissorModes; ++j) {
            fBaseInstances[j].fStrokes[i] = cubicStrokesIdx;
            cubicStrokesIdx += fInstanceCounts[j]->fStrokes[i];
        }
    }

    int trianglesIdx = GrSizeDivRoundUp(cubicStrokesIdx * sizeof(CubicStrokeInstance),
                                        sizeof(TriangleInstance));
    fBaseInstances[0].fTriangles = trianglesIdx;
    fBaseInstances[1].fTriangles =
            fBaseInstances[0].fTriangles + fInstanceCounts[0]->fTriangles;
    int endTrianglesIdx =
            fBaseInstances[1].fTriangles + fInstanceCounts[1]->fTriangles;

    int conicsIdx =
            GrSizeDivRoundUp(endTrianglesIdx * sizeof(TriangleInstance), sizeof(ConicInstance));
    fBaseInstances[0].fConics = conicsIdx;
    fBaseInstances[1].fConics = fBaseInstances[0].fConics + fInstanceCounts[0]->fConics;

    InstanceBufferBuilder builder(onFlushRP, this);
    if (!builder.isMapped()) {
        return false;  // Buffer allocation failed.
    }

    // Now parse the GrCCStrokeGeometry and expand it into the instance buffer.
    int pathIdx = 0;
    int ptsIdx = 0;
    int paramsIdx = 0;
    int normalsIdx = 0;

    const SkTArray<GrCCStrokeGeometry::Parameter, true>& params = fGeometry.params();
    const SkTArray<SkPoint, true>& pts = fGeometry.points();
    const SkTArray<SkVector, true>& normals = fGeometry.normals();

    float miterCapHeightOverWidth=0, conicWeight=0;

    for (Verb verb : fGeometry.verbs()) {
        switch (verb) {
            case Verb::kBeginPath:
                builder.updateCurrentInfo(fPathInfos[pathIdx]);
                ++pathIdx;
                continue;

            case Verb::kLinearStroke:
                builder.appendLinearStroke(&pts[ptsIdx]);
                ++ptsIdx;
                continue;
            case Verb::kQuadraticStroke:
                builder.appendQuadraticStroke(&pts[ptsIdx],
                                              params[paramsIdx++].fNumLinearSegmentsLog2);
                ptsIdx += 2;
                ++normalsIdx;
                continue;
            case Verb::kCubicStroke:
                builder.appendCubicStroke(&pts[ptsIdx], params[paramsIdx++].fNumLinearSegmentsLog2);
                ptsIdx += 3;
                ++normalsIdx;
                continue;

            case Verb::kRoundJoin:
            case Verb::kInternalRoundJoin:
                conicWeight = params[paramsIdx++].fConicWeight;
                // fallthru
            case Verb::kMiterJoin:
                miterCapHeightOverWidth = params[paramsIdx++].fMiterCapHeightOverWidth;
                // fallthru
            case Verb::kBevelJoin:
            case Verb::kInternalBevelJoin:
                builder.appendJoin(verb, pts[ptsIdx], normals[normalsIdx], normals[normalsIdx + 1],
                                   miterCapHeightOverWidth, conicWeight);
                ++normalsIdx;
                continue;

            case Verb::kSquareCap:
            case Verb::kRoundCap:
                builder.appendCap(verb, pts[ptsIdx], normals[normalsIdx]);
                continue;

            case Verb::kEndContour:
                ++ptsIdx;
                ++normalsIdx;
                continue;
        }
        SK_ABORT("Invalid CCPR stroke element.");
    }

    fInstanceBuffer = builder.finish();
    SkASSERT(fPathInfos.count() == pathIdx);
    SkASSERT(pts.count() == ptsIdx);
    SkASSERT(normals.count() == normalsIdx);

    fMeshesBuffer.reserve((1 + fMaxNumScissorSubBatches) * kMaxNumLinearSegmentsLog2);
    fScissorsBuffer.reserve((1 + fMaxNumScissorSubBatches) * kMaxNumLinearSegmentsLog2);
    return true;
}

void GrCCStroker::drawStrokes(GrOpFlushState* flushState, GrCCCoverageProcessor* proc,
                              BatchID batchID, const SkIRect& drawBounds) const {
    using PrimitiveType = GrCCCoverageProcessor::PrimitiveType;
    SkASSERT(fInstanceBuffer);

    if (kEmptyBatchID == batchID) {
        return;
    }
    const Batch& batch = fBatches[batchID];
    int startScissorSubBatch = (!batchID) ? 0 : fBatches[batchID - 1].fEndScissorSubBatch;

    const InstanceTallies* startIndices[2];
    startIndices[(int)GrScissorTest::kDisabled] = (!batchID)
            ? &fZeroTallies : fBatches[batchID - 1].fNonScissorEndInstances;
    startIndices[(int)GrScissorTest::kEnabled] = (!startScissorSubBatch)
            ? &fZeroTallies : fScissorSubBatches[startScissorSubBatch - 1].fEndInstances;

    GrPipeline pipeline(GrScissorTest::kEnabled, SkBlendMode::kPlus,
                        flushState->drawOpArgs().fOutputSwizzle);

    // Draw linear strokes.
    this->appendStrokeMeshesToBuffers(0, batch, startIndices, startScissorSubBatch, drawBounds);
    if (!fMeshesBuffer.empty()) {
        LinearStrokeProcessor linearProc;
        this->flushBufferedMeshesAsStrokes(linearProc, flushState, pipeline, drawBounds);
    }

    // Draw cubic strokes. (Quadratics were converted to cubics for GPU processing.)
    for (int i = 1; i <= kMaxNumLinearSegmentsLog2; ++i) {
        this->appendStrokeMeshesToBuffers(i, batch, startIndices, startScissorSubBatch, drawBounds);
    }
    if (!fMeshesBuffer.empty()) {
        CubicStrokeProcessor cubicProc;
        this->flushBufferedMeshesAsStrokes(cubicProc, flushState, pipeline, drawBounds);
    }

    // Draw triangles.
    proc->reset(PrimitiveType::kTriangles, flushState->resourceProvider());
    this->drawConnectingGeometry<&InstanceTallies::fTriangles>(
            flushState, pipeline, *proc, batch, startIndices, startScissorSubBatch, drawBounds);

    // Draw conics.
    proc->reset(PrimitiveType::kConics, flushState->resourceProvider());
    this->drawConnectingGeometry<&InstanceTallies::fConics>(
            flushState, pipeline, *proc, batch, startIndices, startScissorSubBatch, drawBounds);
}

void GrCCStroker::appendStrokeMeshesToBuffers(int numSegmentsLog2, const Batch& batch,
                                              const InstanceTallies* startIndices[2],
                                              int startScissorSubBatch,
                                              const SkIRect& drawBounds) const {
    // Linear strokes draw a quad. Cubic strokes emit a strip with normals at "numSegments"
    // evenly-spaced points along the curve, plus one more for the final endpoint, plus two more for
    // AA butt caps. (i.e., 2 vertices * (numSegments + 3).)
    int numStripVertices = (0 == numSegmentsLog2) ? 4 : ((1 << numSegmentsLog2) + 3) * 2;

    // Append non-scissored meshes.
    int baseInstance = fBaseInstances[(int)GrScissorTest::kDisabled].fStrokes[numSegmentsLog2];
    int startIdx = startIndices[(int)GrScissorTest::kDisabled]->fStrokes[numSegmentsLog2];
    int endIdx = batch.fNonScissorEndInstances->fStrokes[numSegmentsLog2];
    SkASSERT(endIdx >= startIdx);
    if (int instanceCount = endIdx - startIdx) {
        GrMesh& mesh = fMeshesBuffer.emplace_back(GrPrimitiveType::kTriangleStrip);
        mesh.setInstanced(fInstanceBuffer, instanceCount, baseInstance + startIdx,
                          numStripVertices);
        fScissorsBuffer.push_back(drawBounds);
    }

    // Append scissored meshes.
    baseInstance = fBaseInstances[(int)GrScissorTest::kEnabled].fStrokes[numSegmentsLog2];
    startIdx = startIndices[(int)GrScissorTest::kEnabled]->fStrokes[numSegmentsLog2];
    for (int i = startScissorSubBatch; i < batch.fEndScissorSubBatch; ++i) {
        const ScissorSubBatch& subBatch = fScissorSubBatches[i];
        endIdx = subBatch.fEndInstances->fStrokes[numSegmentsLog2];
        SkASSERT(endIdx >= startIdx);
        if (int instanceCount = endIdx - startIdx) {
            GrMesh& mesh = fMeshesBuffer.emplace_back(GrPrimitiveType::kTriangleStrip);
            mesh.setInstanced(fInstanceBuffer, instanceCount, baseInstance + startIdx,
                              numStripVertices);
            fScissorsBuffer.push_back(subBatch.fScissor);
            startIdx = endIdx;
        }
    }
}

void GrCCStroker::flushBufferedMeshesAsStrokes(const GrPrimitiveProcessor& processor,
                                               GrOpFlushState* flushState,
                                               const GrPipeline& pipeline,
                                               const SkIRect& drawBounds) const {
    SkASSERT(fMeshesBuffer.count() == fScissorsBuffer.count());
    GrPipeline::DynamicStateArrays dynamicStateArrays;
    dynamicStateArrays.fScissorRects = fScissorsBuffer.begin();
    flushState->opsRenderPass()->draw(processor, pipeline, nullptr, &dynamicStateArrays,
                                      fMeshesBuffer.begin(), fMeshesBuffer.count(),
                                      SkRect::Make(drawBounds));
    // Don't call reset(), as that also resets the reserve count.
    fMeshesBuffer.pop_back_n(fMeshesBuffer.count());
    fScissorsBuffer.pop_back_n(fScissorsBuffer.count());
}

template<int GrCCStrokeGeometry::InstanceTallies::* InstanceType>
void GrCCStroker::drawConnectingGeometry(GrOpFlushState* flushState, const GrPipeline& pipeline,
                                         const GrCCCoverageProcessor& processor,
                                         const Batch& batch, const InstanceTallies* startIndices[2],
                                         int startScissorSubBatch,
                                         const SkIRect& drawBounds) const {
    // Append non-scissored meshes.
    int baseInstance = fBaseInstances[(int)GrScissorTest::kDisabled].*InstanceType;
    int startIdx = startIndices[(int)GrScissorTest::kDisabled]->*InstanceType;
    int endIdx = batch.fNonScissorEndInstances->*InstanceType;
    SkASSERT(endIdx >= startIdx);
    if (int instanceCount = endIdx - startIdx) {
        processor.appendMesh(fInstanceBuffer, instanceCount, baseInstance + startIdx,
                             &fMeshesBuffer);
        fScissorsBuffer.push_back(drawBounds);
    }

    // Append scissored meshes.
    baseInstance = fBaseInstances[(int)GrScissorTest::kEnabled].*InstanceType;
    startIdx = startIndices[(int)GrScissorTest::kEnabled]->*InstanceType;
    for (int i = startScissorSubBatch; i < batch.fEndScissorSubBatch; ++i) {
        const ScissorSubBatch& subBatch = fScissorSubBatches[i];
        endIdx = subBatch.fEndInstances->*InstanceType;
        SkASSERT(endIdx >= startIdx);
        if (int instanceCount = endIdx - startIdx) {
            processor.appendMesh(fInstanceBuffer, instanceCount, baseInstance + startIdx,
                                 &fMeshesBuffer);
            fScissorsBuffer.push_back(subBatch.fScissor);
            startIdx = endIdx;
        }
    }

    // Flush the geometry.
    if (!fMeshesBuffer.empty()) {
        SkASSERT(fMeshesBuffer.count() == fScissorsBuffer.count());
        processor.draw(flushState, pipeline, fScissorsBuffer.begin(), fMeshesBuffer.begin(),
                       fMeshesBuffer.count(), SkRect::Make(drawBounds));
        // Don't call reset(), as that also resets the reserve count.
        fMeshesBuffer.pop_back_n(fMeshesBuffer.count());
        fScissorsBuffer.pop_back_n(fScissorsBuffer.count());
    }
}
