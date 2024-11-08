/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathEffect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkPathEffectBase.h"
#include "tests/Test.h"

using namespace skia_private;

DEF_TEST(AsADashTest_noneDash, reporter) {
    sk_sp<SkPathEffect> pe(SkCornerPathEffect::Make(1.0));
    SkPathEffectBase::DashInfo info;

    SkPathEffectBase::DashType dashType = as_PEB(pe)->asADash(&info);
    REPORTER_ASSERT(reporter, SkPathEffectBase::DashType::kNone == dashType);
}

DEF_TEST(AsADashTest_nullInfo, reporter) {
    SkScalar inIntervals[] = { 4.0, 2.0, 1.0, 3.0 };
    const SkScalar phase = 2.0;
    sk_sp<SkPathEffect> pe(SkDashPathEffect::Make(inIntervals, 4, phase));

    SkPathEffectBase::DashType dashType = as_PEB(pe)->asADash(nullptr);
    REPORTER_ASSERT(reporter, SkPathEffectBase::DashType::kDash == dashType);
}

DEF_TEST(AsADashTest_usingDash, reporter) {
    SkScalar inIntervals[] = { 4.0, 2.0, 1.0, 3.0 };
    SkScalar totalIntSum = 10.0;
    const SkScalar phase = 2.0;

    sk_sp<SkPathEffect> pe(SkDashPathEffect::Make(inIntervals, 4, phase));

    SkPathEffectBase::DashInfo info;

    SkPathEffectBase::DashType dashType = as_PEB(pe)->asADash(&info);
    REPORTER_ASSERT(reporter, SkPathEffectBase::DashType::kDash == dashType);
    REPORTER_ASSERT(reporter, 4 == info.fCount);
    REPORTER_ASSERT(reporter, SkScalarMod(phase, totalIntSum) == info.fPhase);

    // Since it is a kDash_DashType, allocate space for the intervals and recall asADash
    AutoTArray<SkScalar> intervals(info.fCount);
    info.fIntervals = intervals.get();
    as_PEB(pe)->asADash(&info);
    REPORTER_ASSERT(reporter, inIntervals[0] == info.fIntervals[0]);
    REPORTER_ASSERT(reporter, inIntervals[1] == info.fIntervals[1]);
    REPORTER_ASSERT(reporter, inIntervals[2] == info.fIntervals[2]);
    REPORTER_ASSERT(reporter, inIntervals[3] == info.fIntervals[3]);

    // Make sure nothing else has changed on us
    REPORTER_ASSERT(reporter, 4 == info.fCount);
    REPORTER_ASSERT(reporter, SkScalarMod(phase, totalIntSum) == info.fPhase);
}
