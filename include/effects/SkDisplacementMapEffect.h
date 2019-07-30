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

class SK_API SkDisplacementMapEffect {
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

    static sk_sp<SkImageFilter> Make(ChannelSelectorType xChannelSelector,
                                     ChannelSelectorType yChannelSelector,
                                     SkScalar scale,
                                     sk_sp<SkImageFilter> displacement,
                                     sk_sp<SkImageFilter> color,
                                     const SkImageFilter::CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> Make(SkColorChannel xChannelSelector,
                                     SkColorChannel yChannelSelector,
                                     SkScalar scale,
                                     sk_sp<SkImageFilter> displacement,
                                     sk_sp<SkImageFilter> color,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

    static void RegisterFlattenables();

private:
    SkDisplacementMapEffect() = delete;
};

#endif
