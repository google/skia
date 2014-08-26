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
    static SkDropShadowImageFilter* Create(SkScalar dx, SkScalar dy,
                                           SkScalar sigmaX, SkScalar sigmaY, SkColor color,
                                           SkImageFilter* input = NULL,
                                           const CropRect* cropRect = NULL,
                                           uint32_t uniqueID = 0) {
        return SkNEW_ARGS(SkDropShadowImageFilter, (dx, dy, sigmaX, sigmaY,
                                                    color, input, cropRect, uniqueID));
    }
    virtual void computeFastBounds(const SkRect&, SkRect*) const SK_OVERRIDE;
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDropShadowImageFilter)

protected:
    SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor,
                            SkImageFilter* input, const CropRect* cropRect, uint32_t uniqueID);
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    explicit SkDropShadowImageFilter(SkReadBuffer&);
#endif
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    virtual bool onFilterImage(Proxy*, const SkBitmap& source, const Context&, SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                                SkIRect* dst) const SK_OVERRIDE;

private:
    SkScalar fDx, fDy, fSigmaX, fSigmaY;
    SkColor fColor;
    typedef SkImageFilter INHERITED;
};
