/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathEffectBase_DEFINED
#define SkPathEffectBase_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSpan.h"

class SkPath;
class SkStrokeRec;

class SkPathEffectBase : public SkPathEffect {
public:
    SkPathEffectBase() {}

    /** \class PointData

        PointData aggregates all the information needed to draw the point
        primitives returned by an 'asPoints' call.
    */
    class PointData {
    public:
        PointData()
            : fFlags(0)
            , fPoints(nullptr)
            , fNumPoints(0) {
            fSize.set(SK_Scalar1, SK_Scalar1);
            // 'asPoints' needs to initialize/fill-in 'fClipRect' if it sets
            // the kUseClip flag
        }
        ~PointData() {
            delete [] fPoints;
        }

        // TODO: consider using passed-in flags to limit the work asPoints does.
        // For example, a kNoPath flag could indicate don't bother generating
        // stamped solutions.

        // Currently none of these flags are supported.
        enum PointFlags {
            kCircles_PointFlag            = 0x01,   // draw points as circles (instead of rects)
            kUsePath_PointFlag            = 0x02,   // draw points as stamps of the returned path
            kUseClip_PointFlag            = 0x04,   // apply 'fClipRect' before drawing the points
        };

        uint32_t           fFlags;      // flags that impact the drawing of the points
        SkPoint*           fPoints;     // the center point of each generated point
        int                fNumPoints;  // number of points in fPoints
        SkVector           fSize;       // the size to draw the points
        SkRect             fClipRect;   // clip required to draw the points (if kUseClip is set)
        SkPath             fPath;       // 'stamp' to be used at each point (if kUsePath is set)

        SkPath             fFirst;      // If not empty, contains geometry for first point
        SkPath             fLast;       // If not empty, contains geometry for last point

        SkSpan<SkPoint> points() { return {fPoints, fNumPoints}; }
    };

    /**
     *  Does applying this path effect to 'src' yield a set of points? If so,
     *  optionally return the points in 'results'.
     */
    bool asPoints(PointData* results, const SkPath& src,
                          const SkStrokeRec&, const SkMatrix&,
                          const SkRect* cullR) const;

    /**
     *  If the PathEffect can be represented as a dash pattern, asADash will return kDash_DashType
     *  and None otherwise. If a non NULL info is passed in, the various DashInfo will be filled
     *  in if the PathEffect can be a dash pattern. If passed in info has an fCount equal or
     *  greater to that of the effect, it will memcpy the values of the dash intervals into the
     *  info. Thus the general approach will be call asADash once with default info to get DashType
     *  and fCount. If effect can be represented as a dash pattern, allocate space for the intervals
     *  in info, then call asADash again with the same info and the intervals will get copied in.
     */

    SkFlattenable::Type getFlattenableType() const override {
        return kSkPathEffect_Type;
    }

    static sk_sp<SkPathEffect> Deserialize(const void* data, size_t size,
                                          const SkDeserialProcs* procs = nullptr) {
        return sk_sp<SkPathEffect>(static_cast<SkPathEffect*>(
                                  SkFlattenable::Deserialize(
                                  kSkPathEffect_Type, data, size, procs).release()));
    }

    /**
     * Filter the input path.
     *
     * The CTM parameter is provided for path effects that can use the information.
     * The output of path effects must always be in the original (input) coordinate system,
     * regardless of whether the path effect uses the CTM or not.
     */
    virtual bool onFilterPath(SkPath*, const SkPath&, SkStrokeRec*, const SkRect*,
                              const SkMatrix& /* ctm */) const = 0;

    /** Path effects *requiring* a valid CTM should override to return true. */
    virtual bool onNeedsCTM() const { return false; }

    virtual bool onAsPoints(PointData*, const SkPath&, const SkStrokeRec&, const SkMatrix&,
                            const SkRect*) const {
        return false;
    }

    enum class DashType {
        kNone, //!< ignores the info parameter
        kDash, //!< fills in all of the info parameter
    };

    struct DashInfo {
        DashInfo() : fIntervals(nullptr), fCount(0), fPhase(0) {}
        DashInfo(SkScalar* intervals, int32_t count, SkScalar phase)
            : fIntervals(intervals), fCount(count), fPhase(phase) {}

        SkScalar*   fIntervals;         //!< Length of on/off intervals for dashed lines
                                        //   Even values represent ons, and odds offs
        int32_t     fCount;             //!< Number of intervals in the dash. Should be even number
        SkScalar    fPhase;             //!< Offset into the dashed interval pattern
                                        //   mod the sum of all intervals
    };

    virtual DashType asADash(DashInfo*) const {
        return DashType::kNone;
    }


    // Compute a conservative bounds for its effect, given the bounds of the path. 'bounds' is
    // both the input and output; if false is returned, fast bounds could not be calculated and
    // 'bounds' is undefined.
    //
    // If 'bounds' is null, performs a dry-run determining if bounds could be computed.
    virtual bool computeFastBounds(SkRect* bounds) const = 0;

    static void RegisterFlattenables();

private:
    using INHERITED = SkPathEffect;
};

////////////////////////////////////////////////////////////////////////////////////////////////

static inline SkPathEffectBase* as_PEB(SkPathEffect* effect) {
    return static_cast<SkPathEffectBase*>(effect);
}

static inline const SkPathEffectBase* as_PEB(const SkPathEffect* effect) {
    return static_cast<const SkPathEffectBase*>(effect);
}

static inline const SkPathEffectBase* as_PEB(const sk_sp<SkPathEffect>& effect) {
    return static_cast<SkPathEffectBase*>(effect.get());
}

static inline sk_sp<SkPathEffectBase> as_PEB_sp(sk_sp<SkPathEffect> effect) {
    return sk_sp<SkPathEffectBase>(static_cast<SkPathEffectBase*>(effect.release()));
}

#endif
