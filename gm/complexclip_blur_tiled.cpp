/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"

#define WIDTH 512
#define HEIGHT 512

namespace skiagm {

class ComplexClipBlurTiledGM : public GM {
public:
    ComplexClipBlurTiledGM() {
    }

protected:
    SkString getName() const override { return SkString("complexclip_blur_tiled"); }

    SkISize getISize() override { return SkISize::Make(WIDTH, HEIGHT); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint blurPaint;
        blurPaint.setImageFilter(SkImageFilters::Blur(5.0f, 5.0f, nullptr));
        const SkScalar tileSize = SkIntToScalar(128);
        SkRect bounds = canvas->getLocalClipBounds();
        int ts = SkScalarCeilToInt(tileSize);
        SkImageInfo info = SkImageInfo::MakeN32Premul(ts, ts);
        auto           tileSurface(ToolUtils::makeSurface(canvas, info));
        SkCanvas* tileCanvas = tileSurface->getCanvas();
        for (SkScalar y = bounds.top(); y < bounds.bottom(); y += tileSize) {
            for (SkScalar x = bounds.left(); x < bounds.right(); x += tileSize) {
                tileCanvas->save();
                tileCanvas->clear(0);
                tileCanvas->translate(-x, -y);
                SkRect rect = SkRect::MakeWH(WIDTH, HEIGHT);
                tileCanvas->saveLayer(&rect, &blurPaint);
                SkRRect rrect = SkRRect::MakeRectXY(rect.makeInset(20, 20), 25, 25);
                tileCanvas->clipRRect(rrect, SkClipOp::kDifference, true);
                SkPaint paint;
                tileCanvas->drawRect(rect, paint);
                tileCanvas->restore();
                tileCanvas->restore();
                canvas->drawImage(tileSurface->makeImageSnapshot().get(), x, y);
            }
        }
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ComplexClipBlurTiledGM;)

}  // namespace skiagm
