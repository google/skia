/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkFastDashing.h"

#include "src/core/SkGeometry.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkTLazy.h"
#include "src/utils/SkArcLengthParameterization.h"
#include "src/utils/SkDashPathPriv.h"

#include <numeric>
#include <vector>

namespace {

class SkFastDashImpl : public SkPathEffectBase {
public:
    SkFastDashImpl(const SkScalar intervals[], int count);

protected:
    void flatten(SkWriteBuffer&) const override;

    bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*) const override;

    bool computeFastBounds(SkRect* bounds) const override { return false; }  // TODO

private:
    typedef SkPathEffect INHERITED;

    SK_FLATTENABLE_HOOKS(SkFastDashImpl)

    std::vector<SkScalar> fIntervals;
    SkScalar fIntervalLength;
    SkScalar fInitialDashLength;
    int fInitialDashIndex;
};

SkFastDashImpl::SkFastDashImpl(const SkScalar intervals[], int count)
        : fIntervals(intervals, intervals + count) {
    SkDashPath::CalcDashParameters(0, intervals, count, &fInitialDashLength, &fInitialDashIndex,
                                   &fIntervalLength, nullptr);
}

void SkFastDashImpl::flatten(SkWriteBuffer&) const {}

sk_sp<SkFlattenable> SkFastDashImpl::CreateProc(SkReadBuffer& buffer) { return nullptr; }

bool SkFastDashImpl::onFilterPath(SkPath* dst,
                                  const SkPath& src,
                                  SkStrokeRec*,
                                  const SkRect*) const {
    const int count = fIntervals.size();
    SkASSERT(count % 2 == 0);

    int index = fInitialDashIndex;
    float dlen = fIntervals[index];

    SkArcLengthParameterization param(src);
    SkArcLengthSegmentIter iter(param);
    const float totalLen = param.totalLength();
    while (iter.currU() < 1.0f) {
        const float du = dlen / totalLen;
        iter.getSegment(du, dst);

        // next interval
        index = (index + 1) % fIntervals.size();
        dlen = fIntervals[index];

        // skip gaps
        if (index % 2 == 1) {
            iter.advance(dlen / totalLen);
            index = (index + 1) % fIntervals.size();
            dlen = fIntervals[index];
        }
    }

    return true;
}

}  // namespace

//////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkFastDashing::Make(const SkScalar intervals[], int count) {
    if (!SkDashPath::ValidDashPath(0, intervals, count)) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkFastDashImpl(intervals, count));
}
