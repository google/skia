
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDiscretePathEffect_DEFINED
#define SkDiscretePathEffect_DEFINED

#include "SkPathEffect.h"

/** \class SkDiscretePathEffect

    This path effect chops a path into discrete segments, and randomly displaces them.
*/
class SkDiscretePathEffect : public SkPathEffect {
public:
    /** Break the path into segments of segLength length, and randomly move the endpoints
        away from the original path by a maximum of deviation.
        Note: works on filled or framed paths
    */
    SkDiscretePathEffect(SkScalar segLength, SkScalar deviation);

    // overrides for SkPathEffect
    //  This method is not exported to java.
    virtual bool filterPath(SkPath* dst, const SkPath& src, SkScalar* width);

    // overrides for SkFlattenable
    //  This method is not exported to java.
    virtual Factory getFactory();
    //  This method is not exported to java.
    virtual void flatten(SkFlattenableWriteBuffer&);

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer&);

protected:
    SkDiscretePathEffect(SkFlattenableReadBuffer&);

private:
    SkScalar fSegLength, fPerterb;
    
    typedef SkPathEffect INHERITED;
};

#endif

