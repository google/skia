/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkFlattenable.h"

#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

class SkModeColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make(SkColor color, SkBlendMode mode) {
        return sk_sp<SkColorFilter>(new SkModeColorFilter(color, mode));
    }

    bool asColorMode(SkColor*, SkBlendMode*) const override;
    uint32_t getFlags() const override;

    Factory getFactory() const override { return CreateProc; }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrContext*, const GrColorSpaceInfo&) const override;
#endif

protected:
    SkModeColorFilter(SkColor color, SkBlendMode mode);

    void flatten(SkWriteBuffer&) const override;

    void onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        bool shaderIsOpaque) const override;

    sk_sp<SkColorFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

private:
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);
    friend class SkFlattenable::PrivateInitializer;

    SkColor     fColor;
    SkBlendMode fMode;

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

#endif
