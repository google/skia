/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

#include "src/core/SkColorFilterBase.h"

class SkModeColorFilter : public SkColorFilterBase {
public:
    static sk_sp<SkColorFilter> Make(SkColor color, SkBlendMode mode) {
        return sk_sp<SkColorFilter>(new SkModeColorFilter(color, mode));
    }

    bool onIsAlphaUnchanged() const override;

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext*, const GrColorInfo&) const override;
#endif

    SK_FLATTENABLE_HOOKS(SkModeColorFilter)

protected:
    SkModeColorFilter(SkColor color, SkBlendMode mode);

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMode(SkColor*, SkBlendMode*) const override;

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;
    skvm::Color onProgram(skvm::Builder*, skvm::Color,
                          const SkColorInfo&, skvm::Uniforms*, SkArenaAlloc*) const override;

private:
    SkColor     fColor;
    SkBlendMode fMode;

    friend class SkColorFilter;

    using INHERITED = SkColorFilterBase;
};

#endif
