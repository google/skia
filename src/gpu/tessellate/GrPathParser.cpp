/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathParser.h"

#include "src/core/SkPathPriv.h"

namespace GrPathParser {

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

int EmitCenterWedges(const SkPath& path, SkPoint* patchData) {
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
                                patchData + vertexCount, lastPoint, parser.startPoint());
                        break;
                    }  // fallthru
                default:
                    continue;

                case SkPathVerb::kConic:
                    SK_ABORT("Conics are not yet supported.");
                    continue;

                case SkPathVerb::kLine:
                    lastPoint = write_line_as_cubic(patchData + vertexCount, lastPoint,
                                                    parser.atPoint(ptsIdx));
                    ++ptsIdx;
                    break;
                case SkPathVerb::kQuad:
                    lastPoint = write_quadratic_as_cubic(patchData + vertexCount, lastPoint,
                                                         parser.atPoint(ptsIdx),
                                                         parser.atPoint(ptsIdx + 1));
                    ptsIdx += 2;
                    break;
                case SkPathVerb::kCubic:
                    lastPoint = write_cubic(patchData + vertexCount, lastPoint,
                                            parser.atPoint(ptsIdx), parser.atPoint(ptsIdx + 1),
                                            parser.atPoint(ptsIdx + 2));
                    ptsIdx += 3;
                    break;
            }
            patchData[vertexCount + 4] = parser.midpoint();
            vertexCount += 5;
        }
    }

    SkASSERT(vertexCount <= MaxPossibleWedgeVertices(path));
    return vertexCount;
}

} // namespace
