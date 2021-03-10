/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkAndroidFrameworkUtils.h"
#include "include/core/SkCanvas.h"
#include "include/utils/SkPaintFilterCanvas.h"
#include "src/core/SkDevice.h"
#include "src/image/SkSurface_Base.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include <log/log.h>

#if SK_SUPPORT_GPU
bool SkAndroidFrameworkUtils::clipWithStencil(SkCanvas* canvas) {
    return canvas->baseDevice()->android_utils_clipWithStencil();
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

void SkAndroidFrameworkUtils::ReplaceClip(SkCanvas* canvas, const SkIRect* rect) {
    SkIRect deviceRestriction;
    if (!rect) {
        if (canvas->fClipRestrictionRect.isEmpty()) {
            deviceRestriction = canvas->imageInfo().bounds();
        } else {
            deviceRestriction = canvas->fClipRestrictionRect;
        }
    } else {
        deviceRestriction = *rect;
    }
    canvas->androidFramework_replaceClip(deviceRestriction);
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

#include "src/shaders/SkShaderBase.h"

(void) as_SB(shader)->onAsAGradient(&privInfo);

SkColor src;
SkBlendMode mode;
(void) as_CFB(filter)->onAsAColorMode(&src, &mode);

bool SkAndroidFrameworkUtils::ShaderAsALinearGradient(const SkShader* shader,
                                                      LinearGradientInfo* info) {
    SkShader::GradientInfo privInfo;

    privInfo.fColorCount   = info->fColorCount;
    privInfo.fColors       = info->fColors;
    privInfo.fColorOffsets = info->fColorOffsets;
    if (as_SB(shader)->onAsAGradient(&privInfo) == SkShader::kLinear_GradientType) {
        info->fColorCount = privInfo.fColorCount;   // may have changed
        return true;
    }
}

bool SkAndroidFrameworkUtils::ColorFilterAsAColorMode(const SkColorFilter* filter,
                                                      SkColor* src, SkBlendMode* mode) {
    return as_CFB(filter)->onAsAColorMode(src, mode);
}

#endif // SK_BUILD_FOR_ANDROID_FRAMEWORK
