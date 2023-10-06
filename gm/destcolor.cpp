/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/effects/SkRuntimeEffect.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

namespace skiagm {

DEF_SIMPLE_GM(destcolor, canvas, 640, 640) {
    // Draw the mandrill.
    canvas->drawImage(ToolUtils::GetResourceAsImage("images/mandrill_512.png"), 0, 0);

    // Now let's add our test effect on top. It reads back the original image and inverts it.
    auto [effect, error] = SkRuntimeEffect::MakeForBlender(SkString(R"(
        half4 main(half4 src, half4 dst) {
            return (half4(1) - dst).rgb1;
        }
    )"));
    SkASSERT(effect);
    SkPaint invertPaint;
    invertPaint.setAntiAlias(true);
    invertPaint.setBlender(effect->makeBlender(nullptr));
    canvas->drawOval(SkRect::MakeLTRB(128, 128, 640, 640), invertPaint);
}

} // namespace skiagm
