
/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawLooper_DEFINED
#define SkDrawLooper_DEFINED

#include "SkFlattenable.h"

class SkCanvas;
class SkPaint;

/** \class SkDrawLooper
    Subclasses of SkDrawLooper can be attached to a SkPaint. Where they are,
    and something is drawn to a canvas with that paint, the looper subclass will
    be called, allowing it to modify the canvas and/or paint for that draw call.
    More than that, via the next() method, the looper can modify the draw to be
    invoked multiple times (hence the name loop-er), allow it to perform effects
    like shadows or frame/fills, that require more than one pass.
*/
class SK_API SkDrawLooper : public SkFlattenable {
public:
    /**
     *  Called right before something is being drawn. This will be followed by
     *  calls to next() until next() returns false.
     */
    virtual void init(SkCanvas*) = 0;

    /**
     *  Called in a loop (after init()). Each time true is returned, the object
     *  is drawn (possibly with a modified canvas and/or paint). When false is
     *  finally returned, drawing for the object stops.
     *
     *  On each call, the paint will be in its original state, but the canvas
     *  will be as it was following the previous call to next() or init().
     *
     *  The implementation must ensure that, when next() finally returns false,
     *  that the canvas has been restored to the state it was initially, before
     *  init() was first called.
     */
    virtual bool next(SkCanvas*, SkPaint* paint) = 0;
    
    /**
     * The fast bounds functions are used to enable the paint to be culled early
     * in the drawing pipeline. If a subclass can support this feature it must
     * return true for the canComputeFastBounds() function.  If that function
     * returns false then computeFastBounds behavior is undefined otherwise it
     * is expected to have the following behavior. Given the parent paint and
     * the parent's bounding rect the subclass must fill in and return the
     * storage rect, where the storage rect is with the union of the src rect
     * and the looper's bounding rect.
     */
    virtual bool canComputeFastBounds(const SkPaint& paint);
    virtual void computeFastBounds(const SkPaint& paint,
                                   const SkRect& src, SkRect* dst);

protected:
    SkDrawLooper() {}
    SkDrawLooper(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

private:
    typedef SkFlattenable INHERITED;
};

#endif
