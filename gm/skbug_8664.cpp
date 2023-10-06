/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

DEF_SIMPLE_GM(skbug_8664, canvas, 830, 550) {
    const struct {
        SkScalar    fSx, fSy, fTx, fTy;
    } xforms[] = {
        { 1, 1, 0, 0 },
        { 0.5f, 0.5f, 530, 0 },
        { 0.25f, 0.25f, 530, 275 },
        { 0.125f, 0.125f, 530, 420 },
    };

    // Must be at least medium to require mipmaps when we downscale the image
    SkSamplingOptions sampling(SkFilterMode::kLinear,
                               SkMipmapMode::kLinear);

    sk_sp<SkImage> image(ToolUtils::GetResourceAsImage("images/mandrill_512.png"));

    SkPaint overlayPaint;
    overlayPaint.setColor(0x80FFFFFF);

    // Make the overlay visible even when the downscaled images fail to render
    canvas->clear(0xFF888888);

    canvas->translate(20, 20);
    for (const auto& xform : xforms) {
        canvas->save();
        canvas->translate(xform.fTx, xform.fTy);
        canvas->scale(xform.fSx, xform.fSy);

        // Draw an image, possibly down sampled, which forces us to generate mipmaps inline
        // on the second iteration.
        canvas->drawImage(image, 0, 0, sampling, nullptr);

        // Draw an overlay that requires the scissor test for its clipping, so that the mipmap
        // generation + scissor interference bug is highlighted in Adreno 330 devices.
        SkRect inner = SkRect::MakeLTRB(32.f, 32.f, 480.f, 480.f);
        SkRect outer = inner.makeOutset(16.f, 16.f);

        // Clip to smaller rectangle
        canvas->save();
        canvas->clipRect(inner);
        // Then apply a rotation and draw a larger rectangle to ensure the clip cannot be dropped
        canvas->rotate(20.f);
        canvas->drawRect(outer, overlayPaint);
        canvas->restore();

        canvas->restore();
    }
}
