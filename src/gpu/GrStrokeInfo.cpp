/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStrokeInfo.h"
#include "GrResourceKey.h"
#include "SkDashPathPriv.h"

bool all_dash_intervals_zero(const SkScalar* intervals, int count) {
    for (int i = 0 ; i < count; ++i) {
        if (intervals[i] != 0) {
            return false;
        }
    }
    return true;
}

bool GrStrokeInfo::applyDashToPath(SkPath* dst, GrStrokeInfo* dstStrokeInfo,
                                   const SkPath& src) const {
    if (this->isDashed()) {
        SkPathEffect::DashInfo info;
        info.fIntervals = fIntervals.get();
        info.fCount = fIntervals.count();
        info.fPhase = fDashPhase;
        GrStrokeInfo filteredStroke(*this, false);
        // Handle the case where all intervals are 0 and we simply drop the dash effect
        if (all_dash_intervals_zero(fIntervals.get(), fIntervals.count())) {
            *dstStrokeInfo = filteredStroke;
            *dst = src;
            return true;
        }
        // See if we can filter the dash into a path on cpu
        if (SkDashPath::FilterDashPath(dst, src, &filteredStroke, nullptr, info)) {
            *dstStrokeInfo = filteredStroke;
            return true;
        }
    }
    return false;
}

void GrStrokeInfo::asUniqueKeyFragment(uint32_t* data) const {
    const int kSkScalarData32Cnt = sizeof(SkScalar) / sizeof(uint32_t);
    enum {
        kStyleBits = 2,
        kJoinBits = 2,
        kCapBits = 32 - kStyleBits - kJoinBits,

        kJoinShift = kStyleBits,
        kCapShift = kJoinShift + kJoinBits,
    };

    static_assert(SkStrokeRec::kStyleCount <= (1 << kStyleBits), "style_shift_will_be_wrong");
    static_assert(SkPaint::kJoinCount <= (1 << kJoinBits), "cap_shift_will_be_wrong");
    static_assert(SkPaint::kCapCount <= (1 << kCapBits), "cap_does_not_fit");
    uint32_t styleKey = this->getStyle();
    if (this->needToApply()) {
        styleKey |= this->getJoin() << kJoinShift;
        styleKey |= this->getCap() << kCapShift;
    }
    int i = 0;
    data[i++] = styleKey;

    // Memcpy the scalar fields. Does not "reinterpret_cast<SkScalar&>(data[i]) = ..." due to
    // scalars having more strict alignment requirements than what data can guarantee. The
    // compiler should optimize memcpys to assignments.
    SkScalar scalar;
    scalar = this->getMiter();
    memcpy(&data[i], &scalar, sizeof(scalar));
    i += kSkScalarData32Cnt;

    scalar = this->getWidth();
    memcpy(&data[i], &scalar, sizeof(scalar));
    i += kSkScalarData32Cnt;

    if (this->isDashed()) {
        SkScalar phase = this->getDashPhase();
        memcpy(&data[i], &phase, sizeof(phase));
        i += kSkScalarData32Cnt;

        int32_t count = this->getDashCount() & static_cast<int32_t>(~1);
        SkASSERT(count == this->getDashCount());
        const SkScalar* intervals = this->getDashIntervals();
        int intervalByteCnt = count * sizeof(SkScalar);
        memcpy(&data[i], intervals, intervalByteCnt);
        // Enable the line below if fields are added after dashing.
        SkDEBUGCODE(i += kSkScalarData32Cnt * count);
    }

    SkASSERT(this->computeUniqueKeyFragmentData32Cnt() == i);
}
