/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"

#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

class SkModeColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make(SkColor color, SkBlendMode mode) {
        return sk_sp<SkColorFilter>(new SkModeColorFilter(color, mode));
    }

    SkColor getColor() const { return fColor; }
    SkPMColor getPMColor() const { return fPMColor; }

    bool asColorMode(SkColor*, SkBlendMode*) const override;
    uint32_t getFlags() const override;

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override;
#endif

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkModeColorFilter)

protected:
    SkModeColorFilter(SkColor color, SkBlendMode mode);

    void flatten(SkWriteBuffer&) const override;

    void onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        bool shaderIsOpaque) const override;

    sk_sp<SkColorFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

private:
    SkColor     fColor;
    SkBlendMode fMode;
    // cache
    SkPMColor   fPMColor;

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

#endif
