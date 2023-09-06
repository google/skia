/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkAndroidFrameworkUtils.h"
#include "include/core/SkCanvas.h"
#include "include/utils/SkPaintFilterCanvas.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkDevice.h"
#include "src/image/SkSurface_Base.h"
#include "src/shaders/SkShaderBase.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include <log/log.h>

#if defined(SK_GANESH)
bool SkAndroidFrameworkUtils::clipWithStencil(SkCanvas* canvas) {
    return canvas->rootDevice()->android_utils_clipWithStencil();
}
#endif

void SkAndroidFrameworkUtils::SafetyNetLog(const char* bugNumber) {
    android_errorWriteLog(0x534e4554, bugNumber);
}

sk_sp<SkSurface> SkAndroidFrameworkUtils::getSurfaceFromCanvas(SkCanvas* canvas) {
    sk_sp<SkSurface> surface(SkSafeRef(canvas->getSurfaceBase()));
    return surface;
}

int SkAndroidFrameworkUtils::SaveBehind(SkCanvas* canvas, const SkRect* subset) {
    return canvas->only_axis_aligned_saveBehind(subset);
}

void SkAndroidFrameworkUtils::ResetClip(SkCanvas* canvas) {
    canvas->internal_private_resetClip();
}

SkCanvas* SkAndroidFrameworkUtils::getBaseWrappedCanvas(SkCanvas* canvas) {
    auto pfc = canvas->internal_private_asPaintFilterCanvas();
    auto result = canvas;
    while (pfc) {
        result = pfc->proxy();
        pfc = result->internal_private_asPaintFilterCanvas();
    }
    return result;
}

bool SkAndroidFrameworkUtils::ShaderAsALinearGradient(SkShader* shader,
                                                      LinearGradientInfo* info) {
    SkASSERT(shader);
    SkTLazy<SkShaderBase::GradientInfo> baseInfo;
    if (info) {
        baseInfo.init();
        baseInfo->fColorCount   = info->fColorCount;
        baseInfo->fColors       = info->fColors;
        baseInfo->fColorOffsets = info->fColorOffsets;
    }
    if (as_SB(shader)->asGradient(baseInfo.getMaybeNull()) != SkShaderBase::GradientType::kLinear) {
        return false;
    }
    if (info) {
        info->fColorCount    = baseInfo->fColorCount;  // this is inout in asGradient()
        info->fPoints[0]     = baseInfo->fPoint[0];
        info->fPoints[1]     = baseInfo->fPoint[1];
        info->fTileMode      = baseInfo->fTileMode;
        info->fGradientFlags = baseInfo->fGradientFlags;
    }
    return true;
}

#endif // SK_BUILD_FOR_ANDROID_FRAMEWORK
