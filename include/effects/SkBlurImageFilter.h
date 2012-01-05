/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBlurImageFilter_DEFINED
#define SkBlurImageFilter_DEFINED

#include "SkImageFilter.h"

class SK_API SkBlurImageFilter : public SkImageFilter {
public:
    SkBlurImageFilter(SkScalar sigmaX, SkScalar sigmaY);

    virtual bool asABlur(SkSize* sigma) const SK_OVERRIDE;

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkBlurImageFilter, (buffer));
    }

    SK_DECLARE_FLATTENABLE_REGISTRAR()

protected:
    explicit SkBlurImageFilter(SkFlattenableReadBuffer& buffer);

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
    virtual void flatten(SkFlattenableWriteBuffer& buffer) SK_OVERRIDE;
    virtual Factory getFactory() SK_OVERRIDE { return CreateProc; }

private:
    SkSize   fSigma;
    typedef SkImageFilter INHERITED;
};

#endif

