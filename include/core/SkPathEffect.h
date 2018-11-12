/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathEffect_DEFINED
#define SkPathEffect_DEFINED

#include "SkFlattenable.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkRect.h"

class SkPath;
class SkStrokeRec;

/** \class SkPathEffect

    SkPathEffect is the base class for objects in the SkPaint that affect
    the geometry of a drawing primitive before it is transformed by the
    canvas' matrix and drawn.

    Dashing is implemented as a subclass of SkPathEffect.
*/
class SK_API SkPathEffect : public SkFlattenable {
public:
    /**
     *  Returns a patheffect that apples each effect (first and second) to the original path,
     *  and returns a path with the sum of these.
     *
     *  result = first(path) + second(path)
     *
     */
    static sk_sp<SkPathEffect> MakeSum(sk_sp<SkPathEffect> first, sk_sp<SkPathEffect> second);

    /**
     *  Returns a patheffect that applies the inner effect to the path, and then applies the
     *  outer effect to the result of the inner's.
     *
     *  result = outer(inner(path))
     */
    static sk_sp<SkPathEffect> MakeCompose(sk_sp<SkPathEffect> outer, sk_sp<SkPathEffect> inner);

    /**
     *  Given a src path (input) and a stroke-rec (input and output), apply
     *  this effect to the src path, returning the new path in dst, and return
     *  true. If this effect cannot be applied, return false and ignore dst
     *  and stroke-rec.
     *
     *  The stroke-rec specifies the initial request for stroking (if any).
     *  The effect can treat this as input only, or it can choose to change
     *  the rec as well. For example, the effect can decide to change the
     *  stroke's width or join, or the effect can change the rec from stroke
     *  to fill (or fill to stroke) in addition to returning a new (dst) path.
     *
     *  If this method returns true, the caller will apply (as needed) the
     *  resulting stroke-rec to dst and then draw.
     */
    bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect* cullR) const;

    /**
     *  Compute a conservative bounds for its effect, given the src bounds.
     *  The baseline implementation just assigns src to dst.
     */
    void computeFastBounds(SkRect* dst, const SkRect& src) const;

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

    enum DashType {
        kNone_DashType, //!< ignores the info parameter
        kDash_DashType, //!< fills in all of the info parameter
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

    DashType asADash(DashInfo* info) const;

    static void RegisterFlattenables();

    static SkFlattenable::Type GetFlattenableType() {
        return kSkPathEffect_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkPathEffect_Type;
    }

    static sk_sp<SkPathEffect> Deserialize(const void* data, size_t size,
                                          const SkDeserialProcs* procs = nullptr) {
        return sk_sp<SkPathEffect>(static_cast<SkPathEffect*>(
                                  SkFlattenable::Deserialize(
                                  kSkPathEffect_Type, data, size, procs).release()));
    }

protected:
    SkPathEffect() {}

    virtual bool onFilterPath(SkPath*, const SkPath&, SkStrokeRec*, const SkRect*) const = 0;
    virtual SkRect onComputeFastBounds(const SkRect& src) const {
        return src;
    }
    virtual bool onAsPoints(PointData*, const SkPath&, const SkStrokeRec&, const SkMatrix&,
                            const SkRect*) const {
        return false;
    }
    virtual DashType onAsADash(DashInfo*) const {
        return kNone_DashType;
    }

private:
    // illegal
    SkPathEffect(const SkPathEffect&);
    SkPathEffect& operator=(const SkPathEffect&);

    typedef SkFlattenable INHERITED;
};

#endif
