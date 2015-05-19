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

class GrUniqueKey;

/*
 * GrStrokeInfo encapsulates all the pertinent infomation regarding the stroke. The SkStrokeRec
 * which holds information on fill style, width, miter, cap, and join. It also holds information
 * about the dash like intervals, count, and phase.
 */
class GrStrokeInfo : public SkStrokeRec {
public:
    GrStrokeInfo(SkStrokeRec::InitStyle style)
        : INHERITED(style)
        , fDashType(SkPathEffect::kNone_DashType) {
    }

    GrStrokeInfo(const GrStrokeInfo& src, bool includeDash = true)
        : INHERITED(src) {
        if (includeDash && src.isDashed()) {
            fDashType = src.fDashType;
            fDashPhase = src.fDashPhase;
            fIntervals.reset(src.getDashCount());
            memcpy(fIntervals.get(), src.fIntervals.get(), fIntervals.count() * sizeof(SkScalar));
        } else {
            fDashType = SkPathEffect::kNone_DashType;
        }
    }

    GrStrokeInfo(const SkPaint& paint, SkPaint::Style styleOverride)
        : INHERITED(paint, styleOverride)
        , fDashType(SkPathEffect::kNone_DashType) {
        this->init(paint);
    }

    explicit GrStrokeInfo(const SkPaint& paint)
        : INHERITED(paint)
        , fDashType(SkPathEffect::kNone_DashType) {
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
        this->INHERITED::operator=(other);
        return *this;
    }

    bool hasEqualEffect(const GrStrokeInfo& other) const {
        if (this->isDashed() != other.isDashed()) {
            return false;
        }
        if (this->isDashed()) {
            if (fDashPhase != other.fDashPhase ||
                fIntervals.count() != other.fIntervals.count() ||
                memcmp(fIntervals.get(), other.fIntervals.get(),
                       fIntervals.count() * sizeof(SkScalar)) != 0) {
                return false;
            }
        }
        return this->INHERITED::hasEqualEffect(other);
    }

    /*
     * This functions takes in a patheffect and updates the dashing information if the path effect
     * is a Dash type. Returns true if the path effect is a dashed effect and we are stroking,
     * otherwise it returns false.
     */
    bool setDashInfo(const SkPathEffect* pe) {
        if (pe && !this->isFillStyle()) {
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
        if (!this->isFillStyle()) {
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
        return (!this->isFillStyle() && SkPathEffect::kDash_DashType == fDashType);
    }

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
    bool applyDashToPath(SkPath* dst, GrStrokeInfo* dstStrokeInfo, const SkPath& src) const;

    /**
     * Computes the length of the data that will be written by asUniqueKeyFragment() function.
     */
    int computeUniqueKeyFragmentData32Cnt() const {
        const int kSkScalarData32Cnt = sizeof(SkScalar) / sizeof(uint32_t);
        // SkStrokeRec data: 32 bits for style+join+cap and 2 scalars for miter and width.
        int strokeKeyData32Cnt = 1 + 2 * kSkScalarData32Cnt;

        if (this->isDashed()) {
            // One scalar for dash phase and one for each dash value.
            strokeKeyData32Cnt += (1 + this->getDashCount()) * kSkScalarData32Cnt;
        }
        return strokeKeyData32Cnt;
    }

    /**
     * Writes the object contents as uint32_t data, to be used with GrUniqueKey.
     * Note: the data written does not encode the length, so care must be taken to ensure
     * that the full unique key data is encoded properly. For example, GrStrokeInfo
     * fragment can be placed last in the sequence, at fixed index.
     */
    void asUniqueKeyFragment(uint32_t*) const;

private:
    // Prevent accidental usage, should use GrStrokeInfo::hasEqualEffect.
    bool hasEqualEffect(const SkStrokeRec& other) const;

    void init(const SkPaint& paint) {
        const SkPathEffect* pe = paint.getPathEffect();
        this->setDashInfo(pe);
    }

    SkPathEffect::DashType fDashType;
    SkScalar               fDashPhase;
    SkAutoSTArray<2, SkScalar> fIntervals;
    typedef SkStrokeRec INHERITED;
};

#endif
