
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
#include "SkStrokeRec.h"
#include "SkTDArray.h"

class SkPath;

/** \class SkPathEffect

    SkPathEffect is the base class for objects in the SkPaint that affect
    the geometry of a drawing primitive before it is transformed by the
    canvas' matrix and drawn.

    Dashing is implemented as a subclass of SkPathEffect.
*/
class SK_API SkPathEffect : public SkFlattenable {
public:
    SK_DECLARE_INST_COUNT(SkPathEffect)

    SkPathEffect() {}

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

    SK_DEFINE_FLATTENABLE_TYPE(SkPathEffect)

protected:
    SkPathEffect(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

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
    SkPairPathEffect(SkPathEffect* pe0, SkPathEffect* pe1);
    virtual ~SkPairPathEffect();

protected:
    SkPairPathEffect(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    // these are visible to our subclasses
    SkPathEffect* fPE0, *fPE1;

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
    SkComposePathEffect(SkPathEffect* outer, SkPathEffect* inner)
        : INHERITED(outer, inner) {}

    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposePathEffect)

protected:
    SkComposePathEffect(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

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
    SkSumPathEffect(SkPathEffect* first, SkPathEffect* second)
        : INHERITED(first, second) {}

    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSumPathEffect)

protected:
    SkSumPathEffect(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

private:
    // illegal
    SkSumPathEffect(const SkSumPathEffect&);
    SkSumPathEffect& operator=(const SkSumPathEffect&);

    typedef SkPairPathEffect INHERITED;
};

#endif
