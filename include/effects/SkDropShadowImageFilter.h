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

    /** @deprecated use another Create function below instead */
    static SkDropShadowImageFilter* Create(SkScalar dx, SkScalar dy,
                                           SkScalar sigmaX, SkScalar sigmaY, SkColor color,
                                           SkImageFilter* input = NULL,
                                           const CropRect* cropRect = NULL,
                                           uint32_t uniqueID = 0) {
        return SkNEW_ARGS(SkDropShadowImageFilter, (dx, dy, sigmaX, sigmaY, color,
                                                    kDrawShadowAndForeground_ShadowMode,
                                                    input, cropRect, uniqueID));
    }

    static SkDropShadowImageFilter* Create(SkScalar dx, SkScalar dy,
                                           SkScalar sigmaX, SkScalar sigmaY, SkColor color,
                                           ShadowMode shadowMode,
                                           SkImageFilter* input,
                                           const CropRect* cropRect,
                                           uint32_t uniqueID) {
        return SkNEW_ARGS(SkDropShadowImageFilter, (dx, dy, sigmaX, sigmaY, color,
                                                    shadowMode, input, cropRect, uniqueID));
    }

    static SkDropShadowImageFilter* Create(SkScalar dx, SkScalar dy,
                                           SkScalar sigmaX, SkScalar sigmaY, SkColor color,
                                           ShadowMode shadowMode) {
        return SkNEW_ARGS(SkDropShadowImageFilter, (dx, dy, sigmaX, sigmaY, color,
                                                    shadowMode, NULL, NULL, 0));
    }

    void computeFastBounds(const SkRect&, SkRect*) const SK_OVERRIDE;
    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDropShadowImageFilter)

protected:
    SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor,
                            ShadowMode shadowMode, SkImageFilter* input, const CropRect* cropRect,
                            uint32_t uniqueID);
    void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    bool onFilterImage(Proxy*, const SkBitmap& source, const Context&, SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                                SkIRect* dst) const SK_OVERRIDE;

private:
    SkScalar fDx, fDy, fSigmaX, fSigmaY;
    SkColor fColor;
    ShadowMode fShadowMode;
    typedef SkImageFilter INHERITED;
};
