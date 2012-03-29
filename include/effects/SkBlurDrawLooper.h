
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBlurDrawLooper_DEFINED
#define SkBlurDrawLooper_DEFINED

#include "SkDrawLooper.h"
#include "SkColor.h"

class SkMaskFilter;
class SkColorFilter;

/** \class SkBlurDrawLooper
    This class draws a shadow of the object (possibly offset), and then draws
    the original object in its original position.
    should there be an option to just draw the shadow/blur layer? webkit?
*/
class SK_API SkBlurDrawLooper : public SkDrawLooper {
public:
    enum BlurFlags {
        kNone_BlurFlag = 0x00,
        /** 
            The blur layer's dx/dy/radius aren't affected by the canvas 
            transform.
        */
        kIgnoreTransform_BlurFlag   = 0x01,
        kOverrideColor_BlurFlag     = 0x02,
        kHighQuality_BlurFlag       = 0x04,
        /** mask for all blur flags */
        kAll_BlurFlag = 0x07
    };

    SkBlurDrawLooper(SkScalar radius, SkScalar dx, SkScalar dy, SkColor color, 
                     uint32_t flags = kNone_BlurFlag);
    virtual ~SkBlurDrawLooper();

    // overrides from SkDrawLooper
    virtual void init(SkCanvas*);
    virtual bool next(SkCanvas*, SkPaint* paint);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurDrawLooper)

protected:
    SkBlurDrawLooper(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    SkMaskFilter*   fBlur;
    SkColorFilter*  fColorFilter;
    SkScalar        fDx, fDy;
    SkColor         fBlurColor;
    uint32_t        fBlurFlags;  

    enum State {
        kBeforeEdge,
        kAfterEdge,
        kDone
    };
    State   fState;
    
    typedef SkDrawLooper INHERITED;
};

#endif
