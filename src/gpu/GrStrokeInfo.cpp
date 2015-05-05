/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStrokeInfo.h"

#include "SkDashPathPriv.h"

bool GrStrokeInfo::applyDash(SkPath* dst, GrStrokeInfo* dstStrokeInfo, const SkPath& src) const {
    if (this->isDashed()) {
        SkPathEffect::DashInfo info;
        info.fIntervals = fIntervals.get();
        info.fCount = fIntervals.count();
        info.fPhase = fDashPhase;
        SkStrokeRec strokeRec = fStroke;
        if (SkDashPath::FilterDashPath(dst, src, &strokeRec, NULL, info)) {
            dstStrokeInfo->fStroke = strokeRec;
            dstStrokeInfo->removeDash();
            return true;
        }
    }
    return false;
}
