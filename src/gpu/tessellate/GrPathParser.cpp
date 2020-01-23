/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathParser.h"

#include "include/private/SkTArray.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrEagerVertexAllocator.h"

static SkPoint lerp(const SkPoint& a, const SkPoint& b, float T) {
    SkASSERT(1 != T);  // The below does not guarantee lerp(a, b, 1) === b.
    return (b - a) * T + a;
}

static SkPoint write_line_as_cubic(SkPoint* data, const SkPoint& p0, const SkPoint& p1) {
    data[0] = p0;
    data[1] = lerp(p0, p1, 1/3.f);
    data[2] = lerp(p0, p1, 2/3.f);
    data[3] = p1;
    return data[3];
}

static SkPoint write_quadratic_as_cubic(SkPoint* data, const SkPoint& p0, const SkPoint& p1,
                                        const SkPoint& p2) {
    data[0] = p0;
    data[1] = lerp(p0, p1, 2/3.f);
    data[2] = lerp(p1, p2, 1/3.f);
    data[3] = p2;
    return data[3];
}

static SkPoint write_cubic(SkPoint* data, const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                           const SkPoint& p3) {
    data[0] = p0;
    data[1] = p1;
    data[2] = p2;
    data[3] = p3;
    return data[3];
}

// SkTPathContourParser specialization that calculates the contour's midpoint.
class MidpointContourParser : public SkTPathContourParser<MidpointContourParser> {
public:
    MidpointContourParser(const SkPath& path) : SkTPathContourParser(path) {}

    bool parseNextContour() {
        if (!this->SkTPathContourParser::parseNextContour()) {
            return false;
        }
        if (fMidpointWeight > 1) {
            fMidpoint *= 1.f / fMidpointWeight;
            fMidpointWeight = 1;
        }
        return true;
    }

    SkPoint midpoint() const { SkASSERT(1 == fMidpointWeight); return fMidpoint; }

private:
    friend class SkTPathContourParser<MidpointContourParser>;

    void resetGeometry(const SkPoint& startPoint) {
        fMidpoint = startPoint;
        fMidpointWeight = 1;
    }

    void geometryTo(SkPathVerb, const SkPoint& endpoint) {
        fMidpoint += endpoint;
        ++fMidpointWeight;
    }

    SkPoint fMidpoint;
    int fMidpointWeight;
};

constexpr int max_wedge_vertex_count(int numPathVerbs) {
    // No initial moveTo, one wedge per verb, plus an implicit close at the end.
    // Each wedge has 5 vertices.
    return (numPathVerbs + 1) * 5;
}

int GrPathParser::EmitCenterWedgePatches(const SkPath& path, GrEagerVertexAllocator* vertexAlloc) {
    int maxVertices = max_wedge_vertex_count(path.countVerbs());
    auto* vertexData = vertexAlloc->lock<SkPoint>(maxVertices);
    if (!vertexData) {
        return 0;
    }

    int vertexCount = 0;
    MidpointContourParser parser(path);
    while (parser.parseNextContour()) {
        int ptsIdx = 0;
        SkPoint lastPoint = parser.startPoint();
        for (int i = 0; i < parser.countVerbs(); ++i) {
            switch (parser.atVerb(i)) {
                case SkPathVerb::kClose:
                case SkPathVerb::kDone:
                    if (parser.startPoint() != lastPoint) {
                        lastPoint = write_line_as_cubic(
                                vertexData + vertexCount, lastPoint, parser.startPoint());
                        break;
                    }  // fallthru
                default:
                    continue;

                case SkPathVerb::kConic:
                    SK_ABORT("Conics are not yet supported.");
                    continue;

                case SkPathVerb::kLine:
                    lastPoint = write_line_as_cubic(vertexData + vertexCount, lastPoint,
                                                    parser.atPoint(ptsIdx));
                    ++ptsIdx;
                    break;
                case SkPathVerb::kQuad:
                    lastPoint = write_quadratic_as_cubic(vertexData + vertexCount, lastPoint,
                                                         parser.atPoint(ptsIdx),
                                                         parser.atPoint(ptsIdx + 1));
                    ptsIdx += 2;
                    break;
                case SkPathVerb::kCubic:
                    lastPoint = write_cubic(vertexData + vertexCount, lastPoint,
                                            parser.atPoint(ptsIdx), parser.atPoint(ptsIdx + 1),
                                            parser.atPoint(ptsIdx + 2));
                    ptsIdx += 3;
                    break;
            }
            vertexData[vertexCount + 4] = parser.midpoint();
            vertexCount += 5;
        }
    }

    vertexAlloc->unlock(vertexCount);
    return vertexCount;
}

// Triangulates the polygon defined by the points in the range [first..last] inclusive.
// Called by InnerPolygonContourParser::emitInnerPolygon() (and recursively).
static int emit_subpolygon(const SkPoint* points, int first, int last, SkPoint* vertexData) {
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
    vertexCount += emit_subpolygon(points, first, mid, vertexData + vertexCount);
    vertexCount += emit_subpolygon(points, mid, last, vertexData + vertexCount);
    return vertexCount;
}

class InnerPolygonContourParser : public SkTPathContourParser<InnerPolygonContourParser> {
public:
    InnerPolygonContourParser(const SkPath& path, int vertexReserveCount)
            : SkTPathContourParser(path)
            , fPolyPoints(vertexReserveCount) {
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
        vertexCount += emit_subpolygon(fPolyPoints.begin(), 0, i1, vertexData + vertexCount);
        vertexCount += emit_subpolygon(fPolyPoints.begin(), i1, i2, vertexData + vertexCount);
        int i3 = fPolyPoints.size();
        fPolyPoints.push_back(fPolyPoints.front());
        vertexCount += emit_subpolygon(fPolyPoints.begin(), i2, i3, vertexData + vertexCount);
        fPolyPoints.pop_back();

        return vertexCount;
    }

    int numCurves() const { return fNumCurves; }

private:
    friend class SkTPathContourParser<InnerPolygonContourParser>;

    void resetGeometry(const SkPoint& startPoint) {
        fPolyPoints.pop_back_n(fPolyPoints.count());
        fPolyPoints.push_back(startPoint);
        fNumCurves = 0;
    }

    void geometryTo(SkPathVerb verb, const SkPoint& endpoint) {
        fPolyPoints.push_back(endpoint);
        if (SkPathVerb::kLine != verb) {
            ++fNumCurves;
        }
    }

    SkSTArray<128, SkPoint> fPolyPoints;
    int fNumCurves;
};

constexpr int max_inner_poly_vertex_count(int numPathVerbs) {
    // No initial moveTo, plus an implicit close at the end; n-2 trianles fill an n-gon.
    // Each triangle has 3 vertices.
    return (numPathVerbs - 1) * 3;
}

int GrPathParser::EmitInnerPolygonTriangles(const SkPath& path,
                                            GrEagerVertexAllocator* vertexAlloc) {
    int maxVertices = max_inner_poly_vertex_count(path.countVerbs());
    InnerPolygonContourParser parser(path, maxVertices);
    auto* vertexData = vertexAlloc->lock<SkPoint>(maxVertices);
    if (!vertexData) {
        return 0;
    }

    int vertexCount = 0;
    while (parser.parseNextContour()) {
        vertexCount += parser.emitInnerPolygon(vertexData + vertexCount);
    }

    vertexAlloc->unlock(vertexCount);
    return vertexCount;
}

int GrPathParser::EmitCubicInstances(const SkPath& path, GrEagerVertexAllocator* vertexAlloc) {
    auto* instanceData = vertexAlloc->lock<std::array<SkPoint, 4>>(path.countVerbs());
    if (!instanceData) {
        return 0;
    }

    int instanceCount = 0;
    SkPath::Iter iter(path, false);
    SkPath::Verb verb;
    SkPoint pts[4];
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (SkPath::kQuad_Verb == verb) {
            write_quadratic_as_cubic(instanceData[instanceCount++].data(), pts[0], pts[1], pts[2]);
            continue;
        }
        if (SkPath::kCubic_Verb == verb) {
            instanceData[instanceCount++] = {pts[0], pts[1], pts[2], pts[3]};
            continue;
        }
    }

    vertexAlloc->unlock(instanceCount);
    return instanceCount;
}
