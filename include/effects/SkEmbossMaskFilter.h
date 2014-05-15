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
class SK_API SkEmbossMaskFilter : public SkMaskFilter {
public:
    struct Light {
        SkScalar    fDirection[3];  // x,y,z
        uint16_t    fPad;
        uint8_t     fAmbient;
        uint8_t     fSpecular;      // exponent, 4.4 right now
    };

    static SkEmbossMaskFilter* Create(SkScalar blurSigma, const Light& light);

    // overrides from SkMaskFilter
    //  This method is not exported to java.
    virtual SkMask::Format getFormat() const SK_OVERRIDE;
    //  This method is not exported to java.
    virtual bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                            SkIPoint* margin) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkEmbossMaskFilter)

protected:
    SkEmbossMaskFilter(SkScalar blurSigma, const Light& light);
    explicit SkEmbossMaskFilter(SkReadBuffer&);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

private:
    Light       fLight;
    SkScalar    fBlurSigma;

    typedef SkMaskFilter INHERITED;
};

#endif
