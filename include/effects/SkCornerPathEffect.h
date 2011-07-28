
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkCornerPathEffect_DEFINED
#define SkCornerPathEffect_DEFINED

#include "SkPathEffect.h"

/** \class SkCornerPathEffect

    SkCornerPathEffect is a subclass of SkPathEffect that can turn sharp corners
    into various treatments (e.g. rounded corners)
*/
class SK_API SkCornerPathEffect : public SkPathEffect {
public:
    /** radius must be > 0 to have an effect. It specifies the distance from each corner
        that should be "rounded".
    */
    SkCornerPathEffect(SkScalar radius);
    virtual ~SkCornerPathEffect();

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
    SkCornerPathEffect(SkFlattenableReadBuffer&);

private:
    SkScalar    fRadius;
    
    typedef SkPathEffect INHERITED;
};

#endif

