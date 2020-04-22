/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMidpointContourParser_DEFINED
#define GrMidpointContourParser_DEFINED

#include "src/core/SkPathPriv.h"

// SkTPathContourParser specialization that also calculates each contour's midpoint.
class GrMidpointContourParser : public SkTPathContourParser<GrMidpointContourParser> {
public:
    GrMidpointContourParser(const SkPath& path) : SkTPathContourParser(path) {}

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

    friend class SkTPathContourParser<GrMidpointContourParser>;
};

 #endif
