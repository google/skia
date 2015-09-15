
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
#include "SkTDArray.h"

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
    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect* cullR) const = 0;

    /**
     *  Compute a conservative bounds for its effect, given the src bounds.
     *  The baseline implementation just assigns src to dst.
     */
    virtual void computeFastBounds(SkRect* dst, const SkRect& src) const;

    /** \class PointData

        PointData aggregates all the information needed to draw the point
        primitives returned by an 'asPoints' call.
    */
    class PointData {
    public:
        PointData()
            : fFlags(0)
            , fPoints(NULL)
            , fNumPoints(0) {
            fSize.set(SK_Scalar1, SK_Scalar1);
            // 'asPoints' needs to initialize/fill-in 'fClipRect' if it sets
            // the kUseClip flag
        };
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
    virtual bool asPoints(PointData* results, const SkPath& src,
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
        DashInfo() : fIntervals(NULL), fCount(0), fPhase(0) {}

        SkScalar*   fIntervals;         //!< Length of on/off intervals for dashed lines
                                        //   Even values represent ons, and odds offs
        int32_t     fCount;             //!< Number of intervals in the dash. Should be even number
        SkScalar    fPhase;             //!< Offset into the dashed interval pattern
                                        //   mod the sum of all intervals
    };

    virtual DashType asADash(DashInfo* info) const;

    SK_TO_STRING_PUREVIRT()
    SK_DEFINE_FLATTENABLE_TYPE(SkPathEffect)

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    /// Override for subclasses as appropriate.
    virtual bool exposedInAndroidJavaAPI() const { return false; }
#endif

protected:
    SkPathEffect() {}

private:
    // illegal
    SkPathEffect(const SkPathEffect&);
    SkPathEffect& operator=(const SkPathEffect&);

    typedef SkFlattenable INHERITED;
};

/** \class SkPairPathEffect

    Common baseclass for Compose and Sum. This subclass manages two pathEffects,
    including flattening them. It does nothing in filterPath, and is only useful
    for managing the lifetimes of its two arguments.
*/
class SkPairPathEffect : public SkPathEffect {
public:
    virtual ~SkPairPathEffect();

protected:
    SkPairPathEffect(SkPathEffect* pe0, SkPathEffect* pe1);

    void flatten(SkWriteBuffer&) const override;

    // these are visible to our subclasses
    SkPathEffect* fPE0, *fPE1;

    SK_TO_STRING_OVERRIDE()

private:
    typedef SkPathEffect INHERITED;
};

/** \class SkComposePathEffect

    This subclass of SkPathEffect composes its two arguments, to create
    a compound pathEffect.
*/
class SkComposePathEffect : public SkPairPathEffect {
public:
    /** Construct a pathEffect whose effect is to apply first the inner pathEffect
        and the the outer pathEffect (e.g. outer(inner(path)))
        The reference counts for outer and inner are both incremented in the constructor,
        and decremented in the destructor.
    */
    static SkComposePathEffect* Create(SkPathEffect* outer, SkPathEffect* inner) {
        return new SkComposePathEffect(outer, inner);
    }

    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposePathEffect)

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool exposedInAndroidJavaAPI() const override { return true; }
#endif

protected:
    SkComposePathEffect(SkPathEffect* outer, SkPathEffect* inner) : INHERITED(outer, inner) {}

private:
    // illegal
    SkComposePathEffect(const SkComposePathEffect&);
    SkComposePathEffect& operator=(const SkComposePathEffect&);

    typedef SkPairPathEffect INHERITED;
};

/** \class SkSumPathEffect

    This subclass of SkPathEffect applies two pathEffects, one after the other.
    Its filterPath() returns true if either of the effects succeeded.
*/
class SkSumPathEffect : public SkPairPathEffect {
public:
    /** Construct a pathEffect whose effect is to apply two effects, in sequence.
        (e.g. first(path) + second(path))
        The reference counts for first and second are both incremented in the constructor,
        and decremented in the destructor.
    */
    static SkSumPathEffect* Create(SkPathEffect* first, SkPathEffect* second) {
        return new SkSumPathEffect(first, second);
    }

    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSumPathEffect)

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool exposedInAndroidJavaAPI() const override { return true; }
#endif

protected:
    SkSumPathEffect(SkPathEffect* first, SkPathEffect* second) : INHERITED(first, second) {}

private:
    // illegal
    SkSumPathEffect(const SkSumPathEffect&);
    SkSumPathEffect& operator=(const SkSumPathEffect&);

    typedef SkPairPathEffect INHERITED;
};

#endif
