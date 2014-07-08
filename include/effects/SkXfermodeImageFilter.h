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
        return SkNEW_ARGS(SkXfermodeImageFilter, (mode, inputs, cropRect));
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkXfermodeImageFilter)

    virtual bool onFilterImage(Proxy* proxy,
                               const SkBitmap& src,
                               const Context& ctx,
                               SkBitmap* dst,
                               SkIPoint* offset) const SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool canFilterImageGPU() const SK_OVERRIDE;
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE;
#endif

protected:
    SkXfermodeImageFilter(SkXfermode* mode, SkImageFilter* inputs[2],
                          const CropRect* cropRect);
    explicit SkXfermodeImageFilter(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

private:
    SkXfermode* fMode;
    typedef SkImageFilter INHERITED;
};

#endif
