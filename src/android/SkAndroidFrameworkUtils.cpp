/*
 * Copyright 2017 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAndroidFrameworkUtils.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "GrStyle.h"
#include "GrClip.h"
#include "GrRenderTargetContext.h"
#include "GrUserStencilSettings.h"
#include "effects/GrDisableColorXP.h"

#ifdef SK_BUILD_FOR_ANDROID

void SkAndroidFrameworkUtils::clipWithStencil(SkCanvas* canvas) {
    SkRegion clipRegion;
    canvas->temporary_internal_getRgnClip(&clipRegion);
    if (clipRegion.isEmpty()) {
        return;
    }
    SkBaseDevice* device = canvas->getDevice();
    if (!device) {
        return;
    }
    GrRenderTargetContext* rtc = device->accessRenderTargetContext();
    if (!rtc) {
        return;
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
}

#endif // SK_BUILD_FOR_ANDROID

