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

    GrStrokeInfo& operator=(const GrStrokeInfo& other) {
        fStroke = other.fStroke;
        fDashInfo = other.fDashInfo;
        fDashType = other.fDashType;
        fIntervals.reset(other.dashCount());
        memcpy(fIntervals.get(), other.fIntervals.get(), other.dashCount() * sizeof(SkScalar));
        return *this;
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
        if (pe && !fStroke.isFillStyle()) {
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

    bool isFillStyle() const { return fStroke.isFillStyle(); }

    int32_t dashCount() const {
        return fDashInfo.fCount;
    }

    void removeDash() {
        fDashType = SkPathEffect::kNone_DashType;
    }
    
    const SkPathEffect::DashInfo& getDashInfo() const { return fDashInfo; }

    /** Applies the dash to a path, if the stroke info has dashing.
     * @return true if the dash ingwas applied (dst and dstStrokeInfo will be modified).
     *         false if the stroke info did not have dashing. The dst and dstStrokeInfo
     *               will be unmodified. The stroking in the SkStrokeRec might still
     *               be applicable.
     */
    bool applyDash(SkPath* dst, GrStrokeInfo* dstStrokeInfo, const SkPath& src) const;

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
