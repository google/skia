/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDisplacementMapEffect_DEFINED
#define SkDisplacementMapEffect_DEFINED

#include "include/core/SkImageFilter.h"

enum class SkColorChannel;

class SK_API SkDisplacementMapEffect : public SkImageFilter {
public:

    // DEPRECATED - Use SkColorChannel instead.
    enum ChannelSelectorType {
        kUnknown_ChannelSelectorType,
        kR_ChannelSelectorType,
        kG_ChannelSelectorType,
        kB_ChannelSelectorType,
        kA_ChannelSelectorType,

        kLast_ChannelSelectorType = kA_ChannelSelectorType
    };

    ~SkDisplacementMapEffect() override;

    static sk_sp<SkImageFilter> Make(ChannelSelectorType xChannelSelector,
                                     ChannelSelectorType yChannelSelector,
                                     SkScalar scale,
                                     sk_sp<SkImageFilter> displacement,
                                     sk_sp<SkImageFilter> color,
                                     const CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> Make(SkColorChannel xChannelSelector,
                                     SkColorChannel yChannelSelector,
                                     SkScalar scale,
                                     sk_sp<SkImageFilter> displacement,
                                     sk_sp<SkImageFilter> color,
                                     const CropRect* cropRect = nullptr);

    SkRect computeFastBounds(const SkRect& src) const override;

    virtual SkIRect onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                   MapDirection, const SkIRect* inputRect) const override;
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

    SkDisplacementMapEffect(SkColorChannel xChannelSelector,
                            SkColorChannel yChannelSelector,
                            SkScalar scale, sk_sp<SkImageFilter> inputs[2],
                            const CropRect* cropRect);
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkDisplacementMapEffect)

    SkColorChannel fXChannelSelector;
    SkColorChannel fYChannelSelector;
    SkScalar fScale;
    typedef SkImageFilter INHERITED;
    const SkImageFilter* getDisplacementInput() const { return getInput(0); }
    const SkImageFilter* getColorInput() const { return getInput(1); }
};

#endif
