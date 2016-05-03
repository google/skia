/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStyle.h"

void GrStyle::initPathEffect(sk_sp<SkPathEffect> pe) {
    if (!pe) {
        fDashInfo.fType = SkPathEffect::kNone_DashType;
        return;
    }
    SkPathEffect::DashInfo info;
    if (SkPathEffect::kDash_DashType == pe->asADash(&info)) {
        if (fStrokeRec.getStyle() == SkStrokeRec::kFill_Style) {
            fPathEffect.reset(nullptr);
        } else {
            fPathEffect = std::move(pe);
            fDashInfo.fType = SkPathEffect::kDash_DashType;
            fDashInfo.fIntervals.reset(info.fCount);
            fDashInfo.fPhase = info.fPhase;
            info.fIntervals = fDashInfo.fIntervals.get();
            pe->asADash(&info);
            return;
        }
    } else {
        fPathEffect = std::move(pe);
    }
    fDashInfo.fType = SkPathEffect::kNone_DashType;
    fDashInfo.fIntervals.reset(0);
}
