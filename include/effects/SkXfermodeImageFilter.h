/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermodeImageFilter_DEFINED
#define SkXfermodeImageFilter_DEFINED

#include "SkBlendMode.h"
#include "SkImageFilter.h"

class SkXfermode;

/**
 * This filter takes an xfermode, and uses it to composite the foreground
 * over the background.  If foreground or background is NULL, the input
 * bitmap (src) is used instead.
 */
class SK_API SkXfermodeImageFilter {
public:
    static sk_sp<SkImageFilter> Make(SkBlendMode, sk_sp<SkImageFilter> background,
                                     sk_sp<SkImageFilter> foreground,
                                     const SkImageFilter::CropRect* cropRect);
    static sk_sp<SkImageFilter> Make(SkBlendMode mode, sk_sp<SkImageFilter> background) {
        return Make(mode, std::move(background), nullptr, nullptr);
    }

    static sk_sp<SkImageFilter> MakeArithmetic(float k1, float k2, float k3, float k4,
                                               bool enforcePMColor,
                                               sk_sp<SkImageFilter> background,
                                               sk_sp<SkImageFilter> foreground,
                                               const SkImageFilter::CropRect* cropRect);
    static sk_sp<SkImageFilter> MakeArithmetic(float k1, float k2, float k3, float k4,
                                               bool enforcePMColor,
                                               sk_sp<SkImageFilter> background) {
        return MakeArithmetic(k1, k2, k3, k4, enforcePMColor, std::move(background),
                              nullptr, nullptr);
    }

#ifdef SK_SUPPORT_LEGACY_XFERMODE_OBJECT
    static sk_sp<SkImageFilter> Make(sk_sp<SkXfermode> mode, sk_sp<SkImageFilter> background,
                                     sk_sp<SkImageFilter> foreground,
                                     const SkImageFilter::CropRect* cropRect);
    static sk_sp<SkImageFilter> Make(sk_sp<SkXfermode> mode, sk_sp<SkImageFilter> background) {
        return Make(std::move(mode), std::move(background), nullptr, nullptr);
    }

#endif

#ifdef SK_SUPPORT_LEGACY_XFERMODE_PTR
    static SkImageFilter* Create(SkXfermode* mode, SkImageFilter* background,
                                 SkImageFilter* foreground = NULL,
                                 const SkImageFilter::CropRect* cropRect = NULL) {
        return Make(sk_ref_sp(mode), 
                    sk_ref_sp(background),
                    sk_ref_sp(foreground),
                    cropRect).release();
    }
#endif
#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static sk_sp<SkImageFilter> Make(sk_sp<SkXfermode> mode, SkImageFilter* background,
                                     SkImageFilter* foreground,
                                     const SkImageFilter::CropRect* cropRect) {
        return Make(std::move(mode),
                    sk_ref_sp(background),
                    sk_ref_sp(foreground),
                    cropRect);
    }
    static sk_sp<SkImageFilter> Make(sk_sp<SkXfermode> mode, SkImageFilter* background) {
        return Make(std::move(mode), sk_ref_sp(background));
    }
#endif

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP();

private:
    SkXfermodeImageFilter();    // can't instantiate
};

#endif
