/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermodeImageFilter_DEFINED
#define SkXfermodeImageFilter_DEFINED

#include "SkImageFilter.h"

class SkBitmap;
class SkXfermode;

class SK_API SkXfermodeImageFilter : public SkImageFilter {
    /**
     * This filter takes an xfermode, and uses it to composite the foreground
     * over the background.  If foreground or background is NULL, the input
     * bitmap (src) is used instead.
      */

public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkXfermode> mode, SkImageFilter* background,
                                     SkImageFilter* foreground, const CropRect* cropRect);
    static sk_sp<SkImageFilter> Make(sk_sp<SkXfermode> mode, SkImageFilter* background) {
        return Make(std::move(mode), background, nullptr, nullptr);
    }
#ifdef SK_SUPPORT_LEGACY_XFERMODE_PTR
    static SkImageFilter* Create(SkXfermode* mode, SkImageFilter* background,
                                 SkImageFilter* foreground = NULL,
                                 const CropRect* cropRect = NULL) {
        return Make(sk_ref_sp(mode), background, foreground, cropRect).release();
    }
#endif
    
    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkXfermodeImageFilter)

    bool onFilterImageDeprecated(Proxy* proxy,
                                 const SkBitmap& src,
                                 const Context& ctx,
                                 SkBitmap* dst,
                                 SkIPoint* offset) const override;
#if SK_SUPPORT_GPU
    bool canFilterImageGPU() const override;
    bool filterImageGPUDeprecated(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                  SkBitmap* result, SkIPoint* offset) const override;
#endif

protected:
    SkXfermodeImageFilter(sk_sp<SkXfermode> mode, SkImageFilter* inputs[2],
                          const CropRect* cropRect);
    void flatten(SkWriteBuffer&) const override;

private:
    sk_sp<SkXfermode> fMode;
    typedef SkImageFilter INHERITED;
};

#endif
