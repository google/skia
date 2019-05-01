/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/third_party/skcms/skcms.h"

#include <utility>

class PictureShaderCacheGM : public skiagm::GM {
public:
    PictureShaderCacheGM(SkScalar tileSize)
        : fTileSize(tileSize) {
    }

 protected:
    void drawTile(SkCanvas* canvas) {
        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);

        canvas->drawCircle(fTileSize / 4, fTileSize / 4, fTileSize / 4, paint);
        canvas->drawRect(SkRect::MakeXYWH(fTileSize / 2, fTileSize / 2,
                                          fTileSize / 2, fTileSize / 2), paint);

        paint.setColor(SK_ColorRED);
        canvas->drawLine(fTileSize / 2, fTileSize * 1 / 3,
                         fTileSize / 2, fTileSize * 2 / 3, paint);
        canvas->drawLine(fTileSize * 1 / 3, fTileSize / 2,
                         fTileSize * 2 / 3, fTileSize / 2, paint);
    }

    void onOnceBeforeDraw() override {
        SkPictureRecorder recorder;
        SkCanvas* pictureCanvas = recorder.beginRecording(fTileSize, fTileSize, nullptr, 0);
        this->drawTile(pictureCanvas);
        fPicture = recorder.finishRecordingAsPicture();
    }

    SkString onShortName() override {
        return SkString("pictureshadercache");
    }

    SkISize onISize() override {
        return SkISize::Make(100, 100);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setShader(fPicture->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));

        {
            // Render in a funny color space that converts green to yellow.
            skcms_Matrix3x3 greenToYellow = {{
                { 1, 1, 0 },
                { 0, 1, 0 },
                { 0, 0, 1 },
            }};
            sk_sp<SkColorSpace> gty = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                            greenToYellow);
            SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100, std::move(gty));
            sk_sp<SkSurface> surface(SkSurface::MakeRaster(info));
            surface->getCanvas()->drawRect(SkRect::MakeWH(fTileSize, fTileSize), paint);
        }

        // When we draw to the canvas, we should see green because we should *not* reuse the
        // cached picture shader.
        canvas->drawRect(SkRect::MakeWH(fTileSize, fTileSize), paint);
    }

private:
    SkScalar         fTileSize;
    sk_sp<SkPicture> fPicture;
    SkBitmap         fBitmap;

    typedef GM INHERITED;
};

DEF_GM(return new PictureShaderCacheGM(100);)
