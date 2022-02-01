/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendModeBlender_DEFINED
#define SkBlendModeBlender_DEFINED

#include "src/core/SkBlenderBase.h"

class SkBlendModeBlender : public SkBlenderBase {
public:
    SkBlendModeBlender(SkBlendMode mode) : fMode(mode) {}

    skstd::optional<SkBlendMode> asBlendMode() const final { return fMode; }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            std::unique_ptr<GrFragmentProcessor> srcFP,
            std::unique_ptr<GrFragmentProcessor> dstFP,
            const GrFPArgs& fpArgs) const override;
#endif

    SK_FLATTENABLE_HOOKS(SkBlendModeBlender)

private:
    using INHERITED = SkBlenderBase;

    void flatten(SkWriteBuffer& buffer) const override;

    skvm::Color onProgram(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                          const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                          SkArenaAlloc* alloc) const override;

    SkBlendMode fMode;
};

#endif
