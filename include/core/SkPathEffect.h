
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPathEffect_DEFINED
#define SkPathEffect_DEFINED

#include "SkFlattenable.h"

class SkPath;

/** \class SkPathEffect

    SkPathEffect is the base class for objects in the SkPaint that affect
    the geometry of a drawing primitive before it is transformed by the
    canvas' matrix and drawn.

    Dashing is implemented as a subclass of SkPathEffect.
*/
class SK_API SkPathEffect : public SkFlattenable {
public:
    SkPathEffect() {}

    /** Given a src path and a width value, return true if the patheffect
        has produced a new path (dst) and a new width value. If false is returned,
        ignore dst and width.
        On input, width >= 0 means the src should be stroked
        On output, width >= 0 means the dst should be stroked
    */
    virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width) = 0;

    /**
     *  Compute a conservative bounds for its effect, given the src bounds.
     *  The baseline implementation just assigns src to dst.
     */
    virtual void computeFastBounds(SkRect* dst, const SkRect& src);

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

    // overrides
    
    virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width);

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

    // overrides
    virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width);

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

