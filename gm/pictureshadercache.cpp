/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkShader.h"
#include "SkSurface.h"

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
        paint.setShader(SkShader::MakePictureShader(fPicture, SkShader::kRepeat_TileMode,
                                                    SkShader::kRepeat_TileMode, nullptr,
                                                    nullptr));

        {
            // Render in a funny color space that converts green to yellow.
            SkMatrix44 greenToYellow(SkMatrix44::kIdentity_Constructor);
            greenToYellow.setFloat(0, 1, 1.0f);
            sk_sp<SkColorSpace> gty = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
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
