
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkEmbossMaskFilter_DEFINED
#define SkEmbossMaskFilter_DEFINED

#include "SkMaskFilter.h"

/** \class SkEmbossMaskFilter

    This mask filter creates a 3D emboss look, by specifying a light and blur amount.
*/
class SkEmbossMaskFilter : public SkMaskFilter {
public:
    struct Light {
        SkScalar    fDirection[3];  // x,y,z
        uint16_t    fPad;
        uint8_t     fAmbient;
        uint8_t     fSpecular;      // exponent, 4.4 right now
    };

    SkEmbossMaskFilter(const Light& light, SkScalar blurRadius);

    // overrides from SkMaskFilter
    //  This method is not exported to java.
    virtual SkMask::Format getFormat();
    //  This method is not exported to java.
    virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                            SkIPoint* margin);

    // overrides from SkFlattenable

    //  This method is not exported to java.
    virtual Factory getFactory();
    //  This method is not exported to java.
    virtual void flatten(SkFlattenableWriteBuffer&);

protected:
    SkEmbossMaskFilter(SkFlattenableReadBuffer&);

private:
    Light       fLight;
    SkScalar    fBlurRadius;

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer&);
    
    typedef SkMaskFilter INHERITED;
};

#endif

