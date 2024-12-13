/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "modules/skresources/include/SkResources.h"
#include "tools/Resources.h"
#include "tools/viewer/AnimatedImageSlide.h"
#include <cmath>

AnimatedImageSlide::AnimatedImageSlide(const SkString& name, const SkString& path)
    : fPath(path)
{
    fName = name;
}

void AnimatedImageSlide::load(SkScalar w, SkScalar h) {
    fWinSize = {w, h};

    // Try loading both as a resource and as a regular file.
    sk_sp<SkData> data = GetResourceAsData(fPath.c_str());
    if (!data) {
        data = SkData::MakeFromFileName(fPath.c_str());
    }

    fImageAsset = skresources::MultiFrameImageAsset::Make(std::move(data));
}

void AnimatedImageSlide::unload() {
    fImageAsset.reset();
    fTimeBase = 0;
}

void AnimatedImageSlide::draw(SkCanvas* canvas) {
    if (!fImageAsset) {
        return;
    }

    sk_sp<SkImage> frame = fImageAsset->getFrame(fFrameMs * 0.001f);

    SkAutoCanvasRestore acr(canvas, true);
    canvas->translate((fWinSize.width() - frame->width()) / 2,
                      (fWinSize.height() - frame->height()) / 2);

    SkPaint outline_paint;
    outline_paint.setAntiAlias(true);
    outline_paint.setColor(0x80000000);
    outline_paint.setStyle(SkPaint::kStroke_Style);

    const SkRect outline = SkRect::Make(frame->bounds()).makeOutset(1, 1);
    canvas->drawRect(outline, outline_paint);

    canvas->drawImage(frame, 0, 0);
}

bool AnimatedImageSlide::animate(double nanos) {
    if (!fImageAsset || !fImageAsset->isMultiFrame()) {
        return false;
    }

    if (!fTimeBase) {
        fTimeBase = nanos;
    }

    fFrameMs = std::fmod((nanos - fTimeBase) * 0.000001f, fImageAsset->duration());

    return true;
}

DEF_SLIDE( return new AnimatedImageSlide(SkString("AnimatedImage"),
                                         SkString("images/alphabetAnim.gif")); )
