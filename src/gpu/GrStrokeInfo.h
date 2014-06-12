/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeInfo_DEFINED
#define GrStrokeInfo_DEFINED

#include "SkStrokeRec.h"
#include "SkPathEffect.h"

/*
 * GrStrokeInfo encapsulates the data objects that hold all the pertinent infomation
 * regarding the stroke. The two objects are SkStrokeRec which holds information on fill style,
 * width, miter, cap, and join. The second object is DashInfo. This holds information about the
 * dash like intervals, count, and phase.
 */
class GrStrokeInfo {
public: 
    GrStrokeInfo(SkStrokeRec::InitStyle style) :
        fStroke(style), fDashType(SkPathEffect::kNone_DashType) {}

    GrStrokeInfo(const GrStrokeInfo& src, bool includeDash = true) : fStroke(src.fStroke) {
        if (includeDash) {
            fDashInfo = src.fDashInfo;
            fDashType = src.fDashType;
            fIntervals.reset(src.dashCount());
            memcpy(fIntervals.get(), src.fIntervals.get(), src.dashCount() * sizeof(SkScalar));
        } else {
            fDashType = SkPathEffect::kNone_DashType;
        }
    }

    GrStrokeInfo(const SkPaint& paint, SkPaint::Style styleOverride) :
        fStroke(paint, styleOverride), fDashType(SkPathEffect::kNone_DashType) {
        this->init(paint);
    }


    explicit GrStrokeInfo(const SkPaint& paint) :
        fStroke(paint), fDashType(SkPathEffect::kNone_DashType) {
        this->init(paint);
    }

    const SkStrokeRec& getStrokeRec() const { return fStroke; }

    SkStrokeRec* getStrokeRecPtr() { return &fStroke; }

    void setFillStyle() { fStroke.setFillStyle(); }

    /*
     * This functions takes in a patheffect and fills in fDashInfo with the various dashing
     * information if the path effect is a Dash type. Returns true if the path effect is a
     * dashed effect and we are stroking, otherwise it retruns false.
     */
    bool setDashInfo(const SkPathEffect* pe) {
        if (NULL != pe && !fStroke.isFillStyle()) {
            fDashInfo.fIntervals = NULL;
            fDashType = pe->asADash(&fDashInfo);
            if (SkPathEffect::kDash_DashType == fDashType) {
                fIntervals.reset(fDashInfo.fCount);
                fDashInfo.fIntervals = fIntervals.get();
                pe->asADash(&fDashInfo);
                return true;
            }
        }
        return false;
    }

    bool isDashed() const {
        return (!fStroke.isFillStyle() && SkPathEffect::kDash_DashType == fDashType);
    }

    int32_t dashCount() const {
        return fDashInfo.fCount;
    }

    void removeDash() {
        fDashType = SkPathEffect::kNone_DashType;
    }
    
    const SkPathEffect::DashInfo& getDashInfo() const { return fDashInfo; }

private:

    void init(const SkPaint& paint) {
        const SkPathEffect* pe = paint.getPathEffect();
        this->setDashInfo(pe);
    }

    SkStrokeRec            fStroke;
    SkPathEffect::DashType fDashType;
    SkPathEffect::DashInfo fDashInfo;
    SkAutoSTArray<2, SkScalar> fIntervals;
};

#endif
