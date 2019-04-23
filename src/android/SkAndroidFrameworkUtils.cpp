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

#if SK_SUPPORT_GPU
#include "src/gpu/GrClip.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrUserStencilSettings.h"
#include "src/gpu/effects/GrDisableColorXP.h"
#endif //SK_SUPPORT_GPU

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include <log/log.h>

#if SK_SUPPORT_GPU
bool SkAndroidFrameworkUtils::clipWithStencil(SkCanvas* canvas) {
    SkRegion clipRegion;
    canvas->temporary_internal_getRgnClip(&clipRegion);
    if (clipRegion.isEmpty()) {
        return false;
    }
    SkBaseDevice* device = canvas->getDevice();
    if (!device) {
        return false;
    }
    GrRenderTargetContext* rtc = device->accessRenderTargetContext();
    if (!rtc) {
        return false;
    }
    GrPaint grPaint;
    grPaint.setXPFactory(GrDisableColorXPFactory::Get());
    GrNoClip noClip;
    static constexpr GrUserStencilSettings kDrawToStencil(
        GrUserStencilSettings::StaticInit<
            0x1,
            GrUserStencilTest::kAlways,
            0x1,
            GrUserStencilOp::kReplace,
            GrUserStencilOp::kReplace,
            0x1>()
    );
    rtc->drawRegion(noClip, std::move(grPaint), GrAA::kNo, SkMatrix::I(), clipRegion,
                    GrStyle::SimpleFill(), &kDrawToStencil);
    return true;
}
#endif //SK_SUPPORT_GPU

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

SkCanvas* SkAndroidFrameworkUtils::getBaseWrappedCanvas(SkCanvas* canvas) {
    auto pfc = canvas->internal_private_asPaintFilterCanvas();
    auto result = canvas;
    while (pfc) {
        result = pfc->proxy();
        pfc = result->internal_private_asPaintFilterCanvas();
    }
    return result;
}
#endif // SK_BUILD_FOR_ANDROID_FRAMEWORK

