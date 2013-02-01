/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendImageFilter_DEFINED
#define SkBlendImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkBitmap.h"

class SK_API SkBlendImageFilter : public SkImageFilter {
public:
    enum Mode {
      kNormal_Mode,
      kMultiply_Mode,
      kScreen_Mode,
      kDarken_Mode,
      kLighten_Mode,
    };
    SkBlendImageFilter(Mode mode, SkImageFilter* background, SkImageFilter* foreground = NULL);

    ~SkBlendImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlendImageFilter)

    virtual bool onFilterImage(Proxy* proxy,
                               const SkBitmap& src,
                               const SkMatrix& ctm,
                               SkBitmap* dst,
                               SkIPoint* offset) SK_OVERRIDE;
#if SK_SUPPORT_GPU
    virtual bool canFilterImageGPU() const SK_OVERRIDE { return true; }
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, SkBitmap* result) SK_OVERRIDE;
#endif

protected:
    explicit SkBlendImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    Mode fMode;
    typedef SkImageFilter INHERITED;
    SkImageFilter* getBackgroundInput() { return getInput(0); }
    SkImageFilter* getForegroundInput() { return getInput(1); }
};

#endif
