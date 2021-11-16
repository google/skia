/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBlendModeBlender.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
#endif

sk_sp<SkBlender> SkBlender::Mode(SkBlendMode mode) {
#define RETURN_SINGLETON_BLENDER(m)                        \
    case m: {                                              \
        static auto* sBlender = new SkBlendModeBlender{m}; \
        return sk_ref_sp(sBlender);                        \
    }

    switch (mode) {
        RETURN_SINGLETON_BLENDER(SkBlendMode::kClear)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kSrc)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kDst)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kSrcOver)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kDstOver)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kSrcIn)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kDstIn)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kSrcOut)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kDstOut)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kSrcATop)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kDstATop)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kXor)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kPlus)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kModulate)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kScreen)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kOverlay)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kDarken)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kLighten)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kColorDodge)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kColorBurn)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kHardLight)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kSoftLight)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kDifference)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kExclusion)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kMultiply)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kHue)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kSaturation)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kColor)
        RETURN_SINGLETON_BLENDER(SkBlendMode::kLuminosity)
    }

    SkDEBUGFAILF("invalid blend mode %d", (int)mode);
    return nullptr;

#undef RETURN_SINGLETON_BLENDER
}

sk_sp<SkFlattenable> SkBlendModeBlender::CreateProc(SkReadBuffer& buffer) {
    SkBlendMode mode = buffer.read32LE(SkBlendMode::kLastMode);
    return SkBlender::Mode(mode);
}

void SkBlendModeBlender::flatten(SkWriteBuffer& buffer) const {
    buffer.writeInt((int)fMode);
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> SkBlendModeBlender::asFragmentProcessor(
        std::unique_ptr<GrFragmentProcessor> srcFP,
        std::unique_ptr<GrFragmentProcessor> dstFP,
        const GrFPArgs& fpArgs) const {
    return GrBlendFragmentProcessor::Make(std::move(srcFP), std::move(dstFP), fMode);
}
#endif

skvm::Color SkBlendModeBlender::onProgram(skvm::Builder* p, skvm::Color src, skvm::Color dst,
                                          const SkColorInfo& colorInfo, skvm::Uniforms* uniforms,
                                          SkArenaAlloc* alloc) const {
    return p->blend(fMode, src, dst);
}
