/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"

class SkModeColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make(SkColor color, SkBlendMode mode) {
        return sk_sp<SkColorFilter>(new SkModeColorFilter(color, mode));
    }

    uint32_t getFlags() const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo&) const override;
#endif

protected:
    SkModeColorFilter(SkColor color, SkBlendMode mode);

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMode(SkColor*, SkBlendMode*) const override;

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

private:
    SK_FLATTENABLE_HOOKS(SkModeColorFilter)

    SkColor     fColor;
    SkBlendMode fMode;

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

#endif
