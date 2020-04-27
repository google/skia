/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInnerPolygonContourParser_DEFINED
#define GrInnerPolygonContourParser_DEFINED

#include "include/private/SkTArray.h"
#include "src/core/SkPathPriv.h"

// SkTPathContourParser specialization that triangulates the path's inner polygon via recursive
// subdivision. The inner polygons connect the endpoints of each verb. (i.e., they are the path that
// would result from collapsing all curves to single lines.)
class GrInnerPolygonContourParser : public SkTPathContourParser<GrInnerPolygonContourParser> {
public:
    GrInnerPolygonContourParser(const SkPath& path, int vertexReserveCount)
            : SkTPathContourParser(path)
            , fPolyPoints(vertexReserveCount) {
    }

    // Triangulates the polygon defined by the points in the range [first..last] inclusive.
    // Called by InnerPolygonContourParser::emitInnerPolygon() (and recursively).
    static int EmitSubpolygon(const SkPoint* points, int first, int last, SkPoint* vertexData) {
        if (last - first < 2) {
            return 0;
        }

        // For sub-polygons we subdivide the points in two and connect the endpoints.
        int mid = (first + last) / 2;
        vertexData[0] = points[first];
        vertexData[1] = points[mid];
        vertexData[2] = points[last];

        // Emit the sub-polygon at each outer-edge of our new triangle.
        int vertexCount = 3;
        vertexCount += EmitSubpolygon(points, first, mid, vertexData + vertexCount);
        vertexCount += EmitSubpolygon(points, mid, last, vertexData + vertexCount);
        return vertexCount;
    }

    int emitInnerPolygon(SkPoint* vertexData) {
        if (fPolyPoints.size() < 3) {
            return 0;
        }

        // For the first triangle in the polygon, subdivide our points into thirds.
        int i1 = fPolyPoints.size() / 3;
        int i2 = (2 * fPolyPoints.size()) / 3;
        vertexData[0] = fPolyPoints[0];
        vertexData[1] = fPolyPoints[i1];
        vertexData[2] = fPolyPoints[i2];

        // Emit the sub-polygons at all three edges of our first triangle.
        int vertexCount = 3;
        vertexCount += EmitSubpolygon(fPolyPoints.begin(), 0, i1, vertexData + vertexCount);
        vertexCount += EmitSubpolygon(fPolyPoints.begin(), i1, i2, vertexData + vertexCount);
        int i3 = fPolyPoints.size();
        fPolyPoints.push_back(fPolyPoints.front());
        vertexCount += EmitSubpolygon(fPolyPoints.begin(), i2, i3, vertexData + vertexCount);
        fPolyPoints.pop_back();

        return vertexCount;
    }

    int numCountedCurves() const { return fNumCountedCurves; }

private:
    void resetGeometry(const SkPoint& startPoint) {
        fPolyPoints.pop_back_n(fPolyPoints.count());
        fPolyPoints.push_back(startPoint);
    }

    void geometryTo(SkPathVerb verb, const SkPoint& endpoint) {
        fPolyPoints.push_back(endpoint);
        if (SkPathVerb::kLine != verb) {
            ++fNumCountedCurves;
        }
    }

    SkSTArray<128, SkPoint> fPolyPoints;
    int fNumCountedCurves = 0;

    friend class SkTPathContourParser<GrInnerPolygonContourParser>;
};

#endif
