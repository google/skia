/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColor.h"
#include "SkImageFilter.h"
#include "SkScalar.h"

class SK_API SkDropShadowImageFilter : public SkImageFilter {
public:
    SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigma, SkColor, SkImageFilter* input = NULL);
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDropShadowImageFilter)

protected:
    explicit SkDropShadowImageFilter(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;
    virtual bool onFilterImage(Proxy*, const SkBitmap& source, const SkMatrix&, SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;

private:
    SkScalar fDx, fDy, fSigma;
    SkColor fColor;
    typedef SkImageFilter INHERITED;
};
