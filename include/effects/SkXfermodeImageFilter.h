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
    virtual ~SkXfermodeImageFilter();

    static SkXfermodeImageFilter* Create(SkXfermode* mode, SkImageFilter* background,
                                         SkImageFilter* foreground = NULL,
                                         const CropRect* cropRect = NULL) {
        SkImageFilter* inputs[2] = { background, foreground };
        return new SkXfermodeImageFilter(mode, inputs, cropRect);
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkXfermodeImageFilter)

    bool onFilterImage(Proxy* proxy,
                       const SkBitmap& src,
                       const Context& ctx,
                       SkBitmap* dst,
                       SkIPoint* offset) const override;
#if SK_SUPPORT_GPU
    bool canFilterImageGPU() const override;
    bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                        SkBitmap* result, SkIPoint* offset) const override;
#endif

protected:
    SkXfermodeImageFilter(SkXfermode* mode, SkImageFilter* inputs[2],
                          const CropRect* cropRect);
    void flatten(SkWriteBuffer&) const override;

private:
    SkXfermode* fMode;
    typedef SkImageFilter INHERITED;
};

#endif
