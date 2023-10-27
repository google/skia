/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBlendModeBlender.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkNoDestructor.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

sk_sp<SkBlender> SkBlender::Mode(SkBlendMode mode) {
#define RETURN_SINGLETON_BLENDER(m)                            \
    case m: {                                                  \
        static SkNoDestructor<SkBlendModeBlender> sBlender(m); \
        return sk_ref_sp(sBlender.get());                      \
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

bool SkBlendModeBlender::onAppendStages(const SkStageRec& rec) const {
    SkBlendMode_AppendStages(fMode, rec.fPipeline);
    return true;
}

bool SkBlenderBase::affectsTransparentBlack() const {
    if (auto blendMode = this->asBlendMode()) {
        SkBlendModeCoeff src, dst;
        if (SkBlendMode_AsCoeff(*blendMode, &src, &dst)) {
            // If the source is (0,0,0,0), then dst is preserved as long as its coefficient
            // evaluates to 1.0. This is true for kOne, kISA, and kISC. Anything else means the
            // blend mode affects transparent black.
            return dst != SkBlendModeCoeff::kOne &&
                   dst != SkBlendModeCoeff::kISA &&
                   dst != SkBlendModeCoeff::kISC;
        } else {
            // An advanced blend mode, which do not affect transparent black
            return false;
        }
    } else {
        // Blenders that aren't blend modes are assumed to modify transparent black.
       return true;
    }
}
