/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrPathParser.h"

#include "include/core/SkPath.h"
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

// SkTPathContoursIter specialization that calculates the contour's midpoint.
class MidpointContoursIter : public SkTPathContoursIter<MidpointContoursIter> {
public:
    MidpointContoursIter(const SkPath& path) : SkTPathContoursIter(path) {}

    bool next() {
        if (!SkTPathContoursIter::next()) {
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
    friend class SkTPathContoursIter;

    void resetGeometry(const SkPoint& startPoint) {
        fMidpoint = startPoint;
        fMidpointWeight = 1;
    }

    void geometryTo(const SkPoint& endpoint) {
        fMidpoint += endpoint;
        ++fMidpointWeight;
    }

    SkPoint fMidpoint;
    int fMidpointWeight;
};

int EmitCenterWedges(const SkPath& path, SkPoint* patchData) {
    int vertexCount = 0;
    MidpointContoursIter contour(path);
    while (contour.next()) {
        int ptsIdx = 0;
        SkPoint lastPoint = contour.startPoint();
        for (int i = 0; i < contour.countVerbs(); ++i) {
            switch (contour.atVerb(i)) {
                case SkPath::kClose_Verb:
                case SkPath::kDone_Verb:
                    if (contour.startPoint() != lastPoint) {
                        lastPoint = write_line_as_cubic(
                                patchData + vertexCount, lastPoint, contour.startPoint());
                        break;
                    }  // fallthru
                default:
                    continue;

                case SkPath::kConic_Verb:
                    SK_ABORT("Conics are not yet supported.");
                    continue;

                case SkPath::kLine_Verb:
                    lastPoint = write_line_as_cubic(patchData + vertexCount, lastPoint,
                                                    contour.atPoint(ptsIdx));
                    ++ptsIdx;
                    break;
                case SkPath::kQuad_Verb:
                    lastPoint = write_quadratic_as_cubic(patchData + vertexCount, lastPoint,
                                                         contour.atPoint(ptsIdx),
                                                         contour.atPoint(ptsIdx + 1));
                    ptsIdx += 2;
                    break;
                case SkPath::kCubic_Verb:
                    lastPoint = write_cubic(patchData + vertexCount, lastPoint,
                                            contour.atPoint(ptsIdx), contour.atPoint(ptsIdx + 1),
                                            contour.atPoint(ptsIdx + 2));
                    ptsIdx += 3;
                    break;
            }
            patchData[vertexCount + 4] = contour.midpoint();
            vertexCount += 5;
        }
    }
    return vertexCount;
}

} // namespace
