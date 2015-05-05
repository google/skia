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
 * GrStrokeInfo encapsulates all the pertinent infomation regarding the stroke. The SkStrokeRec
 * which holds information on fill style, width, miter, cap, and join. It also holds information
 * about the dash like intervals, count, and phase.
 */
class GrStrokeInfo {
public: 
    GrStrokeInfo(SkStrokeRec::InitStyle style) :
        fStroke(style), fDashType(SkPathEffect::kNone_DashType) {}

    GrStrokeInfo(const GrStrokeInfo& src, bool includeDash = true) : fStroke(src.fStroke) {
        if (includeDash && src.isDashed()) {
            fDashType = src.fDashType;
            fDashPhase = src.fDashPhase;
            fIntervals.reset(src.getDashCount());
            memcpy(fIntervals.get(), src.fIntervals.get(), fIntervals.count() * sizeof(SkScalar));
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
        if (other.isDashed()) {
            fDashType = other.fDashType;
            fDashPhase = other.fDashPhase;
            fIntervals.reset(other.getDashCount());
            memcpy(fIntervals.get(), other.fIntervals.get(), fIntervals.count() * sizeof(SkScalar));
        } else {
            this->removeDash();
        }
        fStroke = other.fStroke;
        return *this;
    }

    const SkStrokeRec& getStrokeRec() const { return fStroke; }

    SkStrokeRec* getStrokeRecPtr() { return &fStroke; }

    void setFillStyle() { fStroke.setFillStyle(); }

    /*
     * This functions takes in a patheffect and updates the dashing information if the path effect
     * is a Dash type. Returns true if the path effect is a dashed effect and we are stroking,
     * otherwise it returns false.
     */
    bool setDashInfo(const SkPathEffect* pe) {
        if (pe && !fStroke.isFillStyle()) {
            SkPathEffect::DashInfo dashInfo;
            fDashType = pe->asADash(&dashInfo);
            if (SkPathEffect::kDash_DashType == fDashType) {
                fIntervals.reset(dashInfo.fCount);
                dashInfo.fIntervals = fIntervals.get();
                pe->asADash(&dashInfo);
                fDashPhase = dashInfo.fPhase;
                return true;
            }
        }
        return false;
    }

    /*
     * Like the above, but sets with an explicit SkPathEffect::DashInfo
     */
    bool setDashInfo(const SkPathEffect::DashInfo& info) {
        if (!fStroke.isFillStyle()) {
            SkASSERT(!fStroke.isFillStyle());
            fDashType = SkPathEffect::kDash_DashType;
            fDashPhase = info.fPhase;
            fIntervals.reset(info.fCount);
            for (int i = 0; i < fIntervals.count(); i++) {
                fIntervals[i] = info.fIntervals[i];
            }
            return true;
        }
        return false;
    }

    bool isDashed() const {
        return (!fStroke.isFillStyle() && SkPathEffect::kDash_DashType == fDashType);
    }

    bool isFillStyle() const { return fStroke.isFillStyle(); }

    int32_t getDashCount() const {
        SkASSERT(this->isDashed());
        return fIntervals.count();
    }

    SkScalar getDashPhase() const {
        SkASSERT(this->isDashed());
        return fDashPhase;
    }

    const SkScalar* getDashIntervals() const {
        SkASSERT(this->isDashed());
        return fIntervals.get();
    }

    void removeDash() {
        fDashType = SkPathEffect::kNone_DashType;
    }

    /** Applies the dash to a path, if the stroke info has dashing.
     * @return true if the dashing was applied (dst and dstStrokeInfo will be modified).
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
    SkScalar               fDashPhase;
    SkAutoSTArray<2, SkScalar> fIntervals;
};

#endif
