/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStyle_DEFINED
#define GrStyle_DEFINED

#include "GrTypes.h"
#include "SkPathEffect.h"
#include "SkStrokeRec.h"
#include "SkTemplates.h"

/**
 * Represents the various ways that a GrShape can be styled. It has fill/stroking information
 * as well as an optional path effect. If the path effect represents dashing, the dashing
 * information is extracted from the path effect and stored explicitly.
 *
 * This object does not support stroke-and-fill styling. It is expected that stroking and filling
 * is handled by drawing a stroke and a fill separately.
 *
 * This will replace GrStrokeInfo as GrShape is deployed.
 */
class GrStyle {
public:
    GrStyle() : fStrokeRec(SkStrokeRec::kFill_InitStyle) {
        fDashInfo.fType = SkPathEffect::kNone_DashType;
    }

    GrStyle(const SkStrokeRec& strokeRec, SkPathEffect* pe) : fStrokeRec(strokeRec) {
        SkASSERT(SkStrokeRec::kStrokeAndFill_Style != strokeRec.getStyle());
        this->initPathEffect(pe);
    }

    GrStyle(const GrStyle& that) : fStrokeRec(SkStrokeRec::kFill_InitStyle) {
        *this = that;
    }

    explicit GrStyle(const SkPaint& paint) : fStrokeRec(paint) {
        SkASSERT(SkStrokeRec::kStrokeAndFill_Style != fStrokeRec.getStyle());
        this->initPathEffect(paint.getPathEffect());
    }

    GrStyle& operator=(const GrStyle& that) {
        fPathEffect = that.fPathEffect;
        fDashInfo = that.fDashInfo;
        fStrokeRec = that.fStrokeRec;
        return *this;
    }
    SkPathEffect* pathEffect() const { return fPathEffect.get(); }

    bool isDashed() const { return SkPathEffect::kDash_DashType == fDashInfo.fType; }
    SkScalar dashPhase() const {
        SkASSERT(this->isDashed());
        return fDashInfo.fPhase;
    }
    int dashIntervalCnt() const {
        SkASSERT(this->isDashed());
        return fDashInfo.fIntervals.count();
    }
    const SkScalar* dashIntervals() const {
        SkASSERT(this->isDashed());
        return fDashInfo.fIntervals.get();
    }

    const SkStrokeRec& strokeRec() const { return fStrokeRec; }

private:
    void initPathEffect(SkPathEffect* pe);

    struct DashInfo {
        DashInfo& operator=(const DashInfo& that) {
            fType = that.fType;
            fPhase = that.fPhase;
            fIntervals.reset(that.fIntervals.count());
            memcpy(fIntervals.get(), that.fIntervals.get(),
                   sizeof(SkScalar) * that.fIntervals.count());
            return *this;
        }
        SkPathEffect::DashType      fType;
        SkScalar                    fPhase;
        SkAutoSTArray<4, SkScalar>  fIntervals;
    };

    SkStrokeRec         fStrokeRec;
    sk_sp<SkPathEffect> fPathEffect;
    DashInfo            fDashInfo;
};

#endif
