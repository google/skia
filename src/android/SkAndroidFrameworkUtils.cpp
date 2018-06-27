/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAndroidFrameworkUtils.h"
#include "SkCanvas.h"
#include "SkDevice.h"

#if SK_SUPPORT_GPU
#include "GrStyle.h"
#include "GrClip.h"
#include "GrRenderTargetContext.h"
#include "GrUserStencilSettings.h"
#include "effects/GrDisableColorXP.h"
#endif //SK_SUPPORT_GPU

#ifdef SK_BUILD_FOR_ANDROID

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

#endif // SK_BUILD_FOR_ANDROID

