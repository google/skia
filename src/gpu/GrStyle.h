/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStyle_DEFINED
#define GrStyle_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkStrokeRec.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkPathEffectBase.h"

/**
 * Represents the various ways that a GrStyledShape can be styled. It has fill/stroking information
 * as well as an optional path effect. If the path effect represents dashing, the dashing
 * information is extracted from the path effect and stored explicitly.
 *
 * This will replace GrStrokeInfo as GrStyledShape is deployed.
 */
class GrStyle {
public:
    /**
     * A style object that represents a fill with no path effect.
     * TODO: constexpr with C++14
     */
    static const GrStyle& SimpleFill() {
        static const GrStyle kFill(SkStrokeRec::kFill_InitStyle);
        return kFill;
        }

    /**
     * A style object that represents a hairline stroke with no path effect.
     * TODO: constexpr with C++14
     */
    static const GrStyle& SimpleHairline() {
        static const GrStyle kHairline(SkStrokeRec::kHairline_InitStyle);
        return kHairline;
    }

    enum class Apply {
        kPathEffectOnly,
        kPathEffectAndStrokeRec
    };

    /**
     * Optional flags for computing keys that may remove unnecessary variation in the key due to
     * style settings that don't affect particular classes of geometry.
     */
    enum KeyFlags {
        // The shape being styled has no open contours.
        kClosed_KeyFlag = 0x1,
        // The shape being styled doesn't have any joins and so isn't affected by join type.
        kNoJoins_KeyFlag = 0x2
    };

    /**
     * Computes the key length for a GrStyle. The return will be negative if it cannot be turned
     * into a key. This occurs when there is a path effect that is not a dash. The key can
     * either reflect just the path effect (if one) or the path effect and the strokerec. Note
     * that a simple fill has a zero sized key.
     */
    static int KeySize(const GrStyle&, Apply, uint32_t flags = 0);

    /**
     * Writes a unique key for the style into the provided buffer. This function assumes the buffer
     * has room for at least KeySize() values. It assumes that KeySize() returns a non-negative
     * value for the combination of GrStyle, Apply and flags params. This is written so that the key
     * for just dash application followed by the key for the remaining SkStrokeRec is the same as
     * the key for applying dashing and SkStrokeRec all at once.
     */
    static void WriteKey(uint32_t*, const GrStyle&, Apply, SkScalar scale, uint32_t flags = 0);

    GrStyle() : GrStyle(SkStrokeRec::kFill_InitStyle) {}

    explicit GrStyle(SkStrokeRec::InitStyle initStyle) : fStrokeRec(initStyle) {}

    GrStyle(const SkStrokeRec& strokeRec, sk_sp<SkPathEffect> pe) : fStrokeRec(strokeRec) {
        this->initPathEffect(std::move(pe));
    }

    GrStyle(const GrStyle& that) = default;

    explicit GrStyle(const SkPaint& paint) : fStrokeRec(paint) {
        this->initPathEffect(paint.refPathEffect());
    }

    explicit GrStyle(const SkPaint& paint, SkPaint::Style overrideStyle)
            : fStrokeRec(paint, overrideStyle) {
        this->initPathEffect(paint.refPathEffect());
    }

    GrStyle& operator=(const GrStyle& that) {
        fPathEffect = that.fPathEffect;
        fDashInfo = that.fDashInfo;
        fStrokeRec = that.fStrokeRec;
        return *this;
    }

    void resetToInitStyle(SkStrokeRec::InitStyle fillOrHairline) {
        fDashInfo.reset();
        fPathEffect.reset(nullptr);
        if (SkStrokeRec::kFill_InitStyle == fillOrHairline) {
            fStrokeRec.setFillStyle();
        } else {
            fStrokeRec.setHairlineStyle();
        }
    }

    /** Is this style a fill with no path effect? */
    bool isSimpleFill() const { return fStrokeRec.isFillStyle() && !fPathEffect; }

    /** Is this style a hairline with no path effect? */
    bool isSimpleHairline() const { return fStrokeRec.isHairlineStyle() && !fPathEffect; }

    SkPathEffect* pathEffect() const { return fPathEffect.get(); }
    sk_sp<SkPathEffect> refPathEffect() const { return fPathEffect; }

    bool hasPathEffect() const { return SkToBool(fPathEffect.get()); }

    bool hasNonDashPathEffect() const { return fPathEffect.get() && !this->isDashed(); }

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

    /** Hairline or fill styles without path effects make no alterations to a geometry. */
    bool applies() const {
        return this->pathEffect() || (!fStrokeRec.isFillStyle() && !fStrokeRec.isHairlineStyle());
    }

    static SkScalar MatrixToScaleFactor(const SkMatrix& matrix) {
        // getMaxScale will return -1 if the matrix has perspective. In that case we can use a scale
        // factor of 1. This isn't necessarily a good choice and in the future we might consider
        // taking a bounds here for the perspective case.
        return SkScalarAbs(matrix.getMaxScale());
    }
    /**
     * Applies just the path effect and returns remaining stroke information. This will fail if
     * there is no path effect. dst may or may not have been overwritten on failure. Scale controls
     * geometric approximations made by the path effect. It is typically computed from the view
     * matrix.
     */
    bool SK_WARN_UNUSED_RESULT applyPathEffectToPath(SkPath* dst, SkStrokeRec* remainingStoke,
                                                     const SkPath& src, SkScalar scale) const;

    /**
     * If this succeeds then the result path should be filled or hairlined as indicated by the
     * returned SkStrokeRec::InitStyle value. Will fail if there is no path effect and the
     * strokerec doesn't change the geometry. When this fails the outputs may or may not have
     * been overwritten. Scale controls geometric approximations made by the path effect and
     * stroker. It is typically computed from the view matrix.
     */
    bool SK_WARN_UNUSED_RESULT applyToPath(SkPath* dst, SkStrokeRec::InitStyle* fillOrHairline,
                                           const SkPath& src, SkScalar scale) const;

    /** Given bounds of a path compute the bounds of path with the style applied. */
    void adjustBounds(SkRect* dst, const SkRect& src) const {
        *dst = src;
        auto pe = as_PEB(this->pathEffect());
        if (pe && !pe->computeFastBounds(dst)) {
            // Restore dst == src since ComputeFastBounds leaves it undefined when returning false
            *dst = src;
        }

        // This may not be the correct SkStrokeRec to use if there's a path effect: skbug.com/5299
        // It happens to work for dashing.
        SkScalar radius = fStrokeRec.getInflationRadius();
        dst->outset(radius, radius);
    }

private:
    void initPathEffect(sk_sp<SkPathEffect> pe);

    struct DashInfo {
        DashInfo() : fType(SkPathEffectBase::kNone_DashType) {}
        DashInfo(const DashInfo& that) { *this = that; }
        DashInfo& operator=(const DashInfo& that) {
            fType = that.fType;
            fPhase = that.fPhase;
            fIntervals.reset(that.fIntervals.count());
            sk_careful_memcpy(fIntervals.get(), that.fIntervals.get(),
                              sizeof(SkScalar) * that.fIntervals.count());
            return *this;
        }
        void reset() {
            fType = SkPathEffect::kNone_DashType;
            fIntervals.reset(0);
        }
        SkPathEffect::DashType      fType;
        SkScalar                    fPhase{0};
        SkAutoSTArray<4, SkScalar>  fIntervals;
    };

    bool applyPathEffect(SkPath* dst, SkStrokeRec* strokeRec, const SkPath& src) const;

    SkStrokeRec         fStrokeRec;
    sk_sp<SkPathEffect> fPathEffect;
    DashInfo            fDashInfo;
};

#endif
