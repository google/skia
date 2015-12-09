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
    enum ShadowMode {
        kDrawShadowAndForeground_ShadowMode,
        kDrawShadowOnly_ShadowMode,

        kLast_ShadowMode = kDrawShadowOnly_ShadowMode
    };

    static const int kShadowModeCount = kLast_ShadowMode+1;

    static SkImageFilter* Create(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY,
                                 SkColor color, ShadowMode shadowMode, SkImageFilter* input = NULL,
                                 const CropRect* cropRect = NULL) {
        return new SkDropShadowImageFilter(dx, dy, sigmaX, sigmaY, color, shadowMode, input,
                                           cropRect);
    }

    void computeFastBounds(const SkRect&, SkRect*) const override;
    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDropShadowImageFilter)

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterImage(Proxy*, const SkBitmap& source, const Context&, SkBitmap* result,
                       SkIPoint* loc) const override;
    void onFilterNodeBounds(const SkIRect& src, const SkMatrix&,
                            SkIRect* dst, MapDirection) const override;

private:
    SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor,
                            ShadowMode shadowMode, SkImageFilter* input, const CropRect* cropRect);

    SkScalar fDx, fDy, fSigmaX, fSigmaY;
    SkColor fColor;
    ShadowMode fShadowMode;

    typedef SkImageFilter INHERITED;
};
