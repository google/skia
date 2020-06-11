/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/SkAlignedDashPathEffect.h"
#include "src/effects/SkDashImpl.h"
#include "src/utils/SkDashPathPriv.h"

SkAlignedDashImpl::SkAlignedDashImpl(const SkScalar intervals[], int count)
        : fInitialDashLength(-1), fInitialDashIndex(0), fIntervalLength(0) {
    SkASSERT(intervals);
    SkASSERT(count > 1 && SkIsAlign2(count));

    fIntervals = (SkScalar*)sk_malloc_throw(sizeof(SkScalar) * count);
    fCount = count;
    for (int i = 0; i < count; i++) {
        fIntervals[i] = intervals[i];
    }

    SkDashPath::CalcDashParameters(0, fIntervals, fCount, &fInitialDashLength, &fInitialDashIndex,
                                   &fIntervalLength, nullptr);
}

SkAlignedDashImpl::~SkAlignedDashImpl() { sk_free(fIntervals); }

SkPathEffect::DashType SkAlignedDashImpl::onAsADash(DashInfo* info) const {
    if (info) {
        if (info->fCount >= fCount && info->fIntervals) {
            memcpy(info->fIntervals, fIntervals, fCount * sizeof(SkScalar));
        }
        info->fCount = fCount;
        info->fPhase = 0;
        info->fAlignment = DashAlignment::kPathVerbs_DashAlignment;
    }
    return kDash_DashType;
}

bool SkAlignedDashImpl::onFilterPath(SkPath* dst,
                                     const SkPath& src,
                                     SkStrokeRec* rec,
                                     const SkRect* cullRect) const {
    return SkDashPath::InternalFilter(dst, src, rec, cullRect, fIntervals, fCount,
                                      fInitialDashLength, fInitialDashIndex, fIntervalLength,
                                      DashAlignment::kPathVerbs_DashAlignment);
}

void SkAlignedDashImpl::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalarArray(fIntervals, fCount);
}

sk_sp<SkFlattenable> SkAlignedDashImpl::CreateProc(SkReadBuffer& buffer) {
    const uint32_t count = buffer.getArrayCount();

    if (!buffer.validateCanReadN<SkScalar>(count)) {
        return nullptr;
    }

    SkAutoSTArray<32, SkScalar> intervals(count);
    if (buffer.readScalarArray(intervals.get(), count)) {
        return SkAlignedDashPathEffect::Make(intervals.get(), SkToInt(count));
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathEffect> SkAlignedDashPathEffect::Make(const SkScalar intervals[], int count) {
    if (!SkDashPath::ValidDashPath(0, intervals, count)) {
        return nullptr;
    }
    return sk_sp<SkPathEffect>(new SkAlignedDashImpl(intervals, count));
}
