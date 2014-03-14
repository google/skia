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
    static SkDropShadowImageFilter* Create(SkScalar dx, SkScalar dy, SkScalar sigma,
                                           SkColor color, SkImageFilter* input = NULL) {
        return SkNEW_ARGS(SkDropShadowImageFilter, (dx, dy, sigma, color, input));
    }
    static SkDropShadowImageFilter* Create(SkScalar dx, SkScalar dy,
                                           SkScalar sigmaX, SkScalar sigmaY, SkColor color,
                                           SkImageFilter* input = NULL,
                                           const CropRect* cropRect = NULL) {
        return SkNEW_ARGS(SkDropShadowImageFilter, (dx, dy, sigmaX, sigmaY,
                                                    color, input, cropRect));
    }
    virtual void computeFastBounds(const SkRect&, SkRect*) const SK_OVERRIDE;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDropShadowImageFilter)

protected:
    explicit SkDropShadowImageFilter(SkReadBuffer&);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    virtual bool onFilterImage(Proxy*, const SkBitmap& source, const Context&, SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                                SkIRect* dst) const SK_OVERRIDE;

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigma, SkColor,
                            SkImageFilter* input = NULL);
    SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor,
                            SkImageFilter* input = NULL, const CropRect* cropRect = NULL);

private:
    SkScalar fDx, fDy, fSigmaX, fSigmaY;
    SkColor fColor;
    typedef SkImageFilter INHERITED;
};
