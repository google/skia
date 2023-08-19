/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "tools/ToolUtils.h"

namespace skiagm {

constexpr SkRect kSrcImageClip{75, 75, 275, 275};

static sk_sp<SkImage> create_image(SkCanvas* destCanvas) {
    sk_sp<SkSurface> srcSurface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(500, 500));
    SkCanvas* srcCanvas = srcSurface->getCanvas();

    srcCanvas->clear(SK_ColorRED);

    SkPaint paint;
    paint.setColor(0xff00ff00);
    srcCanvas->drawRect(kSrcImageClip, paint);

    constexpr SkScalar kStrokeWidth = 10;
    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(kStrokeWidth);
    stroke.setColor(0xff008800);
    srcCanvas->drawRect(kSrcImageClip.makeInset(kStrokeWidth / 2, kStrokeWidth / 2), stroke);

    return ToolUtils::MakeTextureImage(destCanvas, srcSurface->makeImageSnapshot());
}

/*
 * The purpose of this test is to exercise all three codepaths in skgpu::ganesh::SurfaceDrawContext
 * (drawFilledRect, fillRectToRect, fillRectWithLocalMatrix) that pre-crop filled rects based on the
 * clip.
 *
 * The test creates an image of a green square surrounded by red background, then draws this image
 * in various ways with the red clipped out. The test is successful if there is no visible red
 * background, scissor is never used, and ideally, all the rectangles draw in one GrDrawOp.
 */
class CroppedRectsGM : public GM {
private:
    SkString getName() const override { return SkString("croppedrects"); }
    SkISize getISize() override { return SkISize::Make(500, 500); }

    void onDraw(SkCanvas* canvas) override {
        if (!fSrcImage) {
            fSrcImage = create_image(canvas);
            if (fSrcImage) {
                fSrcImageShader = fSrcImage->makeShader(SkSamplingOptions());
            }
        }

        canvas->clear(SK_ColorWHITE);

        {
            // skgpu::ganesh::SurfaceDrawContext::drawFilledRect.
            SkAutoCanvasRestore acr(canvas, true);
            SkPaint paint;
            paint.setShader(fSrcImageShader);
            canvas->clipRect(kSrcImageClip);
            canvas->drawPaint(paint);
        }

        {
            // skgpu::ganesh::SurfaceDrawContext::fillRectToRect.
            SkAutoCanvasRestore acr(canvas, true);
            SkRect drawRect = SkRect::MakeXYWH(350, 100, 100, 300);
            canvas->clipRect(drawRect);
            canvas->drawImageRect(fSrcImage.get(),
                                  kSrcImageClip.makeOutset(0.5f * kSrcImageClip.width(),
                                                           kSrcImageClip.height()),
                                  drawRect.makeOutset(0.5f * drawRect.width(), drawRect.height()),
                                  SkSamplingOptions(), nullptr,
                                  SkCanvas::kStrict_SrcRectConstraint);
        }

        {
            // skgpu::ganesh::SurfaceDrawContext::fillRectWithLocalMatrix.
            SkAutoCanvasRestore acr(canvas, true);
            SkPath path = SkPath::Line(
                   {kSrcImageClip.fLeft - kSrcImageClip.width(), kSrcImageClip.centerY()},
                   {kSrcImageClip.fRight + 3 * kSrcImageClip.width(), kSrcImageClip.centerY()});
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(2 * kSrcImageClip.height());
            paint.setShader(fSrcImageShader);
            canvas->translate(23, 301);
            canvas->scale(300 / kSrcImageClip.width(), 100 / kSrcImageClip.height());
            canvas->translate(-kSrcImageClip.left(), -kSrcImageClip.top());
            canvas->clipRect(kSrcImageClip);
            canvas->drawPath(path, paint);
        }

        // TODO: assert the draw target only has one op in the post-MDB world.
    }

    sk_sp<SkImage> fSrcImage;
    sk_sp<SkShader> fSrcImageShader;

    using INHERITED = GM;
};

DEF_GM( return new CroppedRectsGM(); )

}  // namespace skiagm
