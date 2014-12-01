/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDisplacementMapEffect_DEFINED
#define SkDisplacementMapEffect_DEFINED

#include "SkImageFilter.h"
#include "SkBitmap.h"

class SK_API SkDisplacementMapEffect : public SkImageFilter {
public:
    enum ChannelSelectorType {
        kUnknown_ChannelSelectorType,
        kR_ChannelSelectorType,
        kG_ChannelSelectorType,
        kB_ChannelSelectorType,
        kA_ChannelSelectorType
    };

    ~SkDisplacementMapEffect();

    static SkDisplacementMapEffect* Create(ChannelSelectorType xChannelSelector,
                                           ChannelSelectorType yChannelSelector,
                                           SkScalar scale, SkImageFilter* displacement,
                                           SkImageFilter* color = NULL,
                                           const CropRect* cropRect = NULL,
                                           uint32_t uniqueID = 0);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDisplacementMapEffect)

    virtual bool onFilterImage(Proxy* proxy,
                               const SkBitmap& src,
                               const Context& ctx,
                               SkBitmap* dst,
                               SkIPoint* offset) const SK_OVERRIDE;
    virtual void computeFastBounds(const SkRect& src, SkRect* dst) const SK_OVERRIDE;

    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                                SkIRect* dst) const SK_OVERRIDE;

#if SK_SUPPORT_GPU
    virtual bool canFilterImageGPU() const SK_OVERRIDE { return true; }
    virtual bool filterImageGPU(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                                SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE;
#endif

protected:
    SkDisplacementMapEffect(ChannelSelectorType xChannelSelector,
                            ChannelSelectorType yChannelSelector,
                            SkScalar scale, SkImageFilter* inputs[2],
                            const CropRect* cropRect,
                            uint32_t uniqueID);
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    explicit SkDisplacementMapEffect(SkReadBuffer& buffer);
#endif
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

private:
    ChannelSelectorType fXChannelSelector;
    ChannelSelectorType fYChannelSelector;
    SkScalar fScale;
    typedef SkImageFilter INHERITED;
    const SkImageFilter* getDisplacementInput() const { return getInput(0); }
    const SkImageFilter* getColorInput() const { return getInput(1); }
};

#endif
