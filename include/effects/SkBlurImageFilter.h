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
    virtual bool asABlur(SkSize* sigma) const;
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkBlurImageFilter, (buffer));
    }
protected:
    explicit SkBlurImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer& buffer);
    virtual Factory getFactory() { return CreateProc; }
private:
    SkSize   fSigma;
    typedef SkImageFilter INHERITED;
};

#endif

