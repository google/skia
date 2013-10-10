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
    SkXfermodeImageFilter(SkXfermode* mode, SkImageFilter* background,
                          SkImageFilter* foreground = NULL, const CropRect* cropRect = NULL);

    virtual ~SkXfermodeImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkXfermodeImageFilter)

    virtual bool onFilterImage(Proxy* proxy,
                               const SkBitmap& src,
                               const SkMatrix& ctm,
                               SkBitmap* dst,
                               SkIPoint* offset) SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool canFilterImageGPU() const SK_OVERRIDE { return !cropRectIsSet(); }
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                                SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;
#endif

protected:
    explicit SkXfermodeImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    SkXfermode* fMode;
    typedef SkImageFilter INHERITED;
};

#endif
