/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrCCStrokeGeometry_DEFINED
#define GrGrCCStrokeGeometry_DEFINED

#include "SkPaint.h"
#include "SkPoint.h"
#include "SkTArray.h"

class SkStrokeRec;

/**
 * This class converts device-space stroked paths into a set of independent strokes, joins, and caps
 * that map directly to coverage-counted GPU instances. Non-hairline strokes can only be drawn with
 * rigid body transforms; we don't yet support skewing the stroke lines themselves.
 */
class GrCCStrokeGeometry {
public:
    static constexpr int kMaxNumLinearSegmentsLog2 = 15;

    GrCCStrokeGeometry(int numSkPoints = 0, int numSkVerbs = 0)
            : fVerbs(numSkVerbs * 5/2)  // Reserve for a 2.5x expansion in verbs. (Joins get their
                                        // own separate verb in our representation.)
            , fParams(numSkVerbs * 3)  // Somewhere around 1-2 params per verb.
            , fPoints(numSkPoints * 5/4)  // Reserve for a 1.25x expansion in points and normals.
            , fNormals(numSkPoints * 5/4) {}

    // A string of verbs and their corresponding, params, points, and normals are a compact
    // representation of what will eventually be independent instances in GPU buffers. When added
    // up, the combined coverage of all these instances will make complete stroked paths.
    enum class Verb : uint8_t {
        kBeginPath,  // Instructs the iterator to advance its stroke width, atlas offset, etc.

        // Independent strokes of a single line or curve, with (antialiased) butt caps on the ends.
        kLinearStroke,
        kQuadraticStroke,
        kCubicStroke,

        // Joins are a triangles that connect the outer corners of two adjoining strokes. Miters have
        // an additional triangle cap on top of the bevel, and round joins have an arc on top.
        kBevelJoin,
        kMiterJoin,
        kRoundJoin,

        kSquareCap,
        kRoundCap,

        kEndContour  // Instructs the iterator to advance its internal point and normal ptrs.
    };

    // Some verbs require additional parameters(s).
    union Parameter {
        // For cubic and quadratic strokes: How many flat line segments to chop the curve into?
        int fNumLinearSegmentsLog2;
        // For miter and round joins: How tall should the triangle cap be on top of the join?
        // (This triangle is the conic control points for a round join.)
        float fMiterCapHeightOverWidth;
        float fConicWeight;  // Round joins only.
    };

    const SkTArray<Verb, true>& verbs() const { SkASSERT(!fInsideContour); return fVerbs; }
    const SkTArray<Parameter, true>& params() const { SkASSERT(!fInsideContour); return fParams; }
    const SkTArray<SkPoint, true>& points() const { SkASSERT(!fInsideContour); return fPoints; }
    const SkTArray<SkVector, true>& normals() const { SkASSERT(!fInsideContour); return fNormals; }

    // These track the numbers of instances required to draw all the recorded strokes.
    struct InstanceTallies {
        int fStrokes[kMaxNumLinearSegmentsLog2 + 1];
        int fTriangles;
        int fConics;

        InstanceTallies operator+(const InstanceTallies&) const;
    };

    void beginPath(const SkStrokeRec&, float strokeDevWidth, InstanceTallies*);
    void moveTo(SkPoint);
    void lineTo(SkPoint);
    void quadraticTo(const SkPoint[3]);
    void cubicTo(const SkPoint[4]);
    void closeContour();  // Connect back to the first point in the contour and exit.
    void capContourAndExit();  // Add endcaps (if any) and exit the contour.

private:
    void lineTo(SkPaint::Join leftJoin, SkPoint);
    void quadraticTo(SkPaint::Join leftJoin, const SkPoint[3], float maxCurvatureT);

    static constexpr float kLeftMaxCurvatureNone = 1;
    static constexpr float kRightMaxCurvatureNone = 0;
    void cubicTo(SkPaint::Join leftJoin, const SkPoint[4], float maxCurvatureT,
                 float leftMaxCurvatureT, float rightMaxCurvatureT);

    // Pushes a new normal to fNormals and records a join, without changing the current position.
    void rotateTo(SkPaint::Join leftJoin, SkVector normal);

    // Records a stroke in fElememts.
    void recordStroke(Verb, int numSegmentsLog2);

    // Records a join in fElememts with the previous stroke, if the cuurent contour is not empty.
    void recordLeftJoinIfNotEmpty(SkPaint::Join, SkVector nextNormal);
    void recordBevelJoin();
    void recordMiterJoin(float miterCapHeightOverWidth);
    void recordRoundJoin(float miterCapHeightOverWidth, float conicWeight);

    void recordCapsIfAny();

    float fCurrStrokeRadius;
    SkPaint::Join fCurrStrokeJoinType;
    SkPaint::Cap fCurrStrokeCapType;
    InstanceTallies* fCurrStrokeTallies = nullptr;

    // We implement miters by placing a triangle-shaped cap on top of a bevel join. This field tells
    // us what the miter limit is, restated in terms of how tall that triangle cap can be.
    float fMiterMaxCapHeightOverWidth;

    // Any curvature on the original curve gets magnified on the outer edge of the stroke,
    // proportional to how thick the stroke radius is. This field tells us the maximum curvature we
    // can tolerate using the current stroke radius, before linearization artifacts begin to appear
    // on the outer edge.
    //
    // (Curvature this strong is quite rare in practice, but when it does happen, we decompose the
    // section with strong curvature into lineTo's with round joins in between.)
    float fMaxCurvatureCosTheta;

    int fCurrContourFirstPtIdx;
    int fCurrContourFirstNormalIdx;

    SkDEBUGCODE(bool fInsideContour = false);

    SkSTArray<128, Verb, true> fVerbs;
    SkSTArray<128, Parameter, true> fParams;
    SkSTArray<128, SkPoint, true> fPoints;
    SkSTArray<128, SkVector, true> fNormals;
};

inline GrCCStrokeGeometry::InstanceTallies GrCCStrokeGeometry::InstanceTallies::operator+(
        const InstanceTallies& t) const {
    InstanceTallies ret;
    for (int i = 0; i <= kMaxNumLinearSegmentsLog2; ++i) {
        ret.fStrokes[i] = fStrokes[i] + t.fStrokes[i];
    }
    ret.fTriangles = fTriangles + t.fTriangles;
    ret.fConics = fConics + t.fConics;
    return ret;
}

#endif
